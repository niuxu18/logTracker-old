static type_list_t list_check;
static type_list_t list_check_copy;

/*
 * Private functions
 */
static int email_config (const char *key, const char *value)
{
	if (0 == strcasecmp (key, "SocketFile")) {
		if (NULL != sock_file)
			free (sock_file);
		sock_file = sstrdup (value);
	}
	else if (0 == strcasecmp (key, "SocketGroup")) {
		if (NULL != sock_group)
			free (sock_group);
		sock_group = sstrdup (value);
	}
	else if (0 == strcasecmp (key, "SocketPerms")) {
		/* the user is responsible for providing reasonable values */
		sock_perms = (int)strtol (value, NULL, 8);
	}
	else if (0 == strcasecmp (key, "MaxConns")) {
		long int tmp = strtol (value, NULL, 0);

		if (tmp < 1) {
			fprintf (stderr, "email plugin: `MaxConns' was set to invalid "
					"value %li, will use default %i.\n",
					tmp, MAX_CONNS);
			ERROR ("email plugin: `MaxConns' was set to invalid "
					"value %li, will use default %i.\n",
					tmp, MAX_CONNS);
			max_conns = MAX_CONNS;
		}
		else if (tmp > MAX_CONNS_LIMIT) {
			fprintf (stderr, "email plugin: `MaxConns' was set to invalid "
					"value %li, will use hardcoded limit %i.\n",
					tmp, MAX_CONNS_LIMIT);
			ERROR ("email plugin: `MaxConns' was set to invalid "
					"value %li, will use hardcoded limit %i.\n",
					tmp, MAX_CONNS_LIMIT);
			max_conns = MAX_CONNS_LIMIT;
		}
		else {
			max_conns = (int)tmp;
		}
	}
	else {
		return -1;
	}
	return 0;
} /* static int email_config (char *, char *) */

/* Increment the value of the given name in the given list by incr. */
static void type_list_incr (type_list_t *list, char *name, int incr)
{
	if (NULL == list->head) {
		list->head = smalloc (sizeof (*list->head));

		list->head->name  = sstrdup (name);
		list->head->value = incr;
		list->head->next  = NULL;

		list->tail = list->head;
	}
	else {
		type_t *ptr;

		for (ptr = list->head; NULL != ptr; ptr = ptr->next) {
			if (0 == strcmp (name, ptr->name))
				break;
		}

		if (NULL == ptr) {
			list->tail->next = smalloc (sizeof (*list->tail->next));
			list->tail = list->tail->next;

			list->tail->name  = sstrdup (name);
			list->tail->value = incr;
			list->tail->next  = NULL;
		}
		else {
			ptr->value += incr;
		}
	}
	return;
} /* static void type_list_incr (type_list_t *, char *) */

static void *collect (void *arg)
{
	collector_t *this = (collector_t *)arg;

	while (1) {
		conn_t *connection;

		pthread_mutex_lock (&conns_mutex);

		while (NULL == conns.head) {
			pthread_cond_wait (&conn_available, &conns_mutex);
		}

		connection = conns.head;
		conns.head = conns.head->next;

		if (NULL == conns.head) {
			conns.tail = NULL;
		}

		pthread_mutex_unlock (&conns_mutex);

		/* make the socket available to the global
		 * thread and connection management */
		this->socket = connection->socket;

		log_debug ("collect: handling connection on fd #%i",
				fileno (this->socket));

		while (42) {
			/* 256 bytes ought to be enough for anybody ;-) */
			char line[256 + 1]; /* line + '\0' */
			int  len = 0;

			errno = 0;
			if (NULL == fgets (line, sizeof (line), this->socket)) {
				if (0 != errno) {
					char errbuf[1024];
					log_err ("collect: reading from socket (fd #%i) "
							"failed: %s", fileno (this->socket),
							sstrerror (errno, errbuf, sizeof (errbuf)));
				}
				break;
			}

			len = strlen (line);
			if (('\n' != line[len - 1]) && ('\r' != line[len - 1])) {
				log_warn ("collect: line too long (> %zu characters): "
						"'%s' (truncated)", sizeof (line) - 1, line);

				while (NULL != fgets (line, sizeof (line), this->socket))
					if (('\n' == line[len - 1]) || ('\r' == line[len - 1]))
						break;
				continue;
			}
			if (len < 3) { /* [a-z] ':' '\n' */
				continue;
			}

			line[len - 1] = 0;

			log_debug ("collect: line = '%s'", line);

			if (':' != line[1]) {
				log_err ("collect: syntax error in line '%s'", line);
				continue;
			}

			if ('e' == line[0]) { /* e:<type>:<bytes> */
				char *ptr  = NULL;
				char *type = strtok_r (line + 2, ":", &ptr);
				char *tmp  = strtok_r (NULL, ":", &ptr);
				int  bytes = 0;

				if (NULL == tmp) {
					log_err ("collect: syntax error in line '%s'", line);
					continue;
				}

				bytes = atoi (tmp);

				pthread_mutex_lock (&count_mutex);
				type_list_incr (&list_count, type, /* increment = */ 1);
				pthread_mutex_unlock (&count_mutex);

				if (bytes > 0) {
					pthread_mutex_lock (&size_mutex);
					type_list_incr (&list_size, type, /* increment = */ bytes);
					pthread_mutex_unlock (&size_mutex);
				}
			}
			else if ('s' == line[0]) { /* s:<value> */
				pthread_mutex_lock (&score_mutex);
				score = (score * (double)score_count + atof (line + 2))
						/ (double)(score_count + 1);
				++score_count;
				pthread_mutex_unlock (&score_mutex);
			}
			else if ('c' == line[0]) { /* c:<type1>[,<type2>,...] */
				char *dummy = line + 2;
				char *endptr = NULL;
				char *type;

				pthread_mutex_lock (&check_mutex);
				while ((type = strtok_r (dummy, ",", &endptr)) != NULL)
				{
					dummy = NULL;
					type_list_incr (&list_check, type, /* increment = */ 1);
				}
				pthread_mutex_unlock (&check_mutex);
			}
			else {
				log_err ("collect: unknown type '%c'", line[0]);
			}
		} /* while (42) */

		log_debug ("Shutting down connection on fd #%i",
				fileno (this->socket));

		fclose (connection->socket);
		free (connection);

		this->socket = NULL;

		pthread_mutex_lock (&available_mutex);
		++available_collectors;
		pthread_mutex_unlock (&available_mutex);

		pthread_cond_signal (&collector_available);
	} /* while (1) */

	pthread_exit ((void *)0);
	return ((void *) 0);
} /* static void *collect (void *) */

static void *open_connection (void __attribute__((unused)) *arg)
{
	struct sockaddr_un addr;

	const char *path  = (NULL == sock_file) ? SOCK_PATH : sock_file;
	const char *group = (NULL == sock_group) ? COLLECTD_GRP_NAME : sock_group;

	/* create UNIX socket */
	errno = 0;
	if (-1 == (connector_socket = socket (PF_UNIX, SOCK_STREAM, 0))) {
		char errbuf[1024];
		disabled = 1;
		log_err ("socket() failed: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		pthread_exit ((void *)1);
	}

	addr.sun_family = AF_UNIX;
	sstrncpy (addr.sun_path, path, (size_t)(UNIX_PATH_MAX - 1));

	errno = 0;
	if (-1 == bind (connector_socket, (struct sockaddr *)&addr,
				offsetof (struct sockaddr_un, sun_path)
					+ strlen(addr.sun_path))) {
		char errbuf[1024];
		disabled = 1;
		close (connector_socket);
		connector_socket = -1;
		log_err ("bind() failed: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		pthread_exit ((void *)1);
	}

	errno = 0;
	if (-1 == listen (connector_socket, 5)) {
		char errbuf[1024];
		disabled = 1;
		close (connector_socket);
		connector_socket = -1;
		log_err ("listen() failed: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		pthread_exit ((void *)1);
	}

	{
		struct group sg;
		struct group *grp;
		char grbuf[2048];
		int status;

		grp = NULL;
		status = getgrnam_r (group, &sg, grbuf, sizeof (grbuf), &grp);
		if (status != 0)
		{
			char errbuf[1024];
			log_warn ("getgrnam_r (%s) failed: %s", group,
					sstrerror (errno, errbuf, sizeof (errbuf)));
		}
		else if (grp == NULL)
		{
			log_warn ("No such group: `%s'", group);
		}
		else
		{
			status = chown (path, (uid_t) -1, grp->gr_gid);
			if (status != 0)
			{
				char errbuf[1024];
				log_warn ("chown (%s, -1, %i) failed: %s",
						path, (int) grp->gr_gid,
						sstrerror (errno, errbuf, sizeof (errbuf)));
			}
		}
	}

	errno = 0;
	if (0 != chmod (path, sock_perms)) {
		char errbuf[1024];
		log_warn ("chmod() failed: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
	}

	{ /* initialize collector threads */
		pthread_attr_t ptattr;

		conns.head = NULL;
		conns.tail = NULL;

		pthread_attr_init (&ptattr);
		pthread_attr_setdetachstate (&ptattr, PTHREAD_CREATE_DETACHED);

		available_collectors = max_conns;

		collectors =
			smalloc (max_conns * sizeof (*collectors));

		for (int i = 0; i < max_conns; ++i) {
			collectors[i] = smalloc (sizeof (*collectors[i]));
			collectors[i]->socket = NULL;

			if (plugin_thread_create (&collectors[i]->thread,
							&ptattr, collect, collectors[i]) != 0) {
				char errbuf[1024];
				log_err ("plugin_thread_create() failed: %s",
						sstrerror (errno, errbuf, sizeof (errbuf)));
				collectors[i]->thread = (pthread_t) 0;
			}
		}

		pthread_attr_destroy (&ptattr);
	}

	while (1) {
		int remote = 0;

		conn_t *connection;

		pthread_mutex_lock (&available_mutex);

		while (0 == available_collectors) {
			pthread_cond_wait (&collector_available, &available_mutex);
		}

		--available_collectors;

		pthread_mutex_unlock (&available_mutex);

		while (42) {
			errno = 0;

			remote = accept (connector_socket, NULL, NULL);
			if (remote == -1) {
				char errbuf[1024];

				if (errno == EINTR)
					continue;

				disabled = 1;
				close (connector_socket);
				connector_socket = -1;
				log_err ("accept() failed: %s",
						 sstrerror (errno, errbuf, sizeof (errbuf)));
				pthread_exit ((void *)1);
			}

			/* access() succeeded. */
			break;
		}

		connection = calloc (1, sizeof (*connection));
		if (connection == NULL)
		{
			close (remote);
			continue;
		}

		connection->socket = fdopen (remote, "r");
		connection->next   = NULL;

		if (NULL == connection->socket) {
			close (remote);
			sfree (connection);
			continue;
		}

		pthread_mutex_lock (&conns_mutex);

		if (NULL == conns.head) {
			conns.head = connection;
			conns.tail = connection;
		}
		else {
			conns.tail->next = connection;
			conns.tail = conns.tail->next;
		}

		pthread_mutex_unlock (&conns_mutex);

		pthread_cond_signal (&conn_available);
	}

	pthread_exit ((void *) 0);
	return ((void *) 0);
} /* static void *open_connection (void *) */

static int email_init (void)
{
	if (plugin_thread_create (&connector, NULL,
				open_connection, NULL) != 0) {
		char errbuf[1024];
		disabled = 1;
		log_err ("plugin_thread_create() failed: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		return (-1);
	}

	return (0);
} /* int email_init */

static void type_list_free (type_list_t *t)
{
	type_t *this;

	this = t->head;
	while (this != NULL)
	{
		type_t *next = this->next;

		sfree (this->name);
		sfree (this);

		this = next;
	}

	t->head = NULL;
	t->tail = NULL;
}

static int email_shutdown (void)
{
	if (connector != ((pthread_t) 0)) {
		pthread_kill (connector, SIGTERM);
		connector = (pthread_t) 0;
	}

	if (connector_socket >= 0) {
		close (connector_socket);
		connector_socket = -1;
	}

	/* don't allow any more connections to be processed */
	pthread_mutex_lock (&conns_mutex);

	available_collectors = 0;

	if (collectors != NULL) {
		for (int i = 0; i < max_conns; ++i) {
			if (collectors[i] == NULL)
				continue;

			if (collectors[i]->thread != ((pthread_t) 0)) {
				pthread_kill (collectors[i]->thread, SIGTERM);
				collectors[i]->thread = (pthread_t) 0;
			}

			if (collectors[i]->socket != NULL) {
				fclose (collectors[i]->socket);
				collectors[i]->socket = NULL;
			}

			sfree (collectors[i]);
		}
		sfree (collectors);
	} /* if (collectors != NULL) */

	pthread_mutex_unlock (&conns_mutex);

	type_list_free (&list_count);
	type_list_free (&list_count_copy);
	type_list_free (&list_size);
	type_list_free (&list_size_copy);
	type_list_free (&list_check);
	type_list_free (&list_check_copy);

	unlink ((NULL == sock_file) ? SOCK_PATH : sock_file);

	sfree (sock_file);
	sfree (sock_group);
	return (0);
} /* static void email_shutdown (void) */

static void email_submit (const char *type, const char *type_instance, gauge_t value)
{
	value_t values[1];
	value_list_t vl = VALUE_LIST_INIT;

	values[0].gauge = value;

	vl.values = values;
	vl.values_len = 1;
	sstrncpy (vl.host, hostname_g, sizeof (vl.host));
	sstrncpy (vl.plugin, "email", sizeof (vl.plugin));
	sstrncpy (vl.type, type, sizeof (vl.type));
	sstrncpy (vl.type_instance, type_instance, sizeof (vl.type_instance));

	plugin_dispatch_values (&vl);
} /* void email_submit */

/* Copy list l1 to list l2. l2 may partly exist already, but it is assumed
 * that neither the order nor the name of any element of either list is
 * changed and no elements are deleted. The values of l1 are reset to zero
 * after they have been copied to l2. */
static void copy_type_list (type_list_t *l1, type_list_t *l2)
{
	type_t *last = NULL;

	for (type_t *ptr1 = l1->head, *ptr2 = l2->head; NULL != ptr1;
			ptr1 = ptr1->next, last = ptr2, ptr2 = ptr2->next) {
		if (NULL == ptr2) {
			ptr2 = smalloc (sizeof (*ptr2));
			ptr2->name = NULL;
			ptr2->next = NULL;

			if (NULL == last) {
				l2->head = ptr2;
			}
			else {
				last->next = ptr2;
			}

			l2->tail = ptr2;
		}

		if (NULL == ptr2->name) {
			ptr2->name = sstrdup (ptr1->name);
		}

		ptr2->value = ptr1->value;
		ptr1->value = 0;
	}
	return;
}

static int email_read (void)
{
	double score_old;
	int score_count_old;

	if (disabled)
		return (-1);

	/* email count */
	pthread_mutex_lock (&count_mutex);

	copy_type_list (&list_count, &list_count_copy);

	pthread_mutex_unlock (&count_mutex);

	for (type_t *ptr = list_count_copy.head; NULL != ptr; ptr = ptr->next) {
		email_submit ("email_count", ptr->name, ptr->value);
	}

	/* email size */
	pthread_mutex_lock (&size_mutex);

	copy_type_list (&list_size, &list_size_copy);

	pthread_mutex_unlock (&size_mutex);

	for (type_t *ptr = list_size_copy.head; NULL != ptr; ptr = ptr->next) {
		email_submit ("email_size", ptr->name, ptr->value);
	}

	/* spam score */
	pthread_mutex_lock (&score_mutex);

	score_old = score;
	score_count_old = score_count;
	score = 0.0;
	score_count = 0;

	pthread_mutex_unlock (&score_mutex);

	if (score_count_old > 0)
		email_submit ("spam_score", "", score_old);

	/* spam checks */
	pthread_mutex_lock (&check_mutex);

	copy_type_list (&list_check, &list_check_copy);

	pthread_mutex_unlock (&check_mutex);

	for (type_t *ptr = list_check_copy.head; NULL != ptr; ptr = ptr->next)
		email_submit ("spam_check", ptr->name, ptr->value);

	return (0);
} /* int email_read */

void module_register (void)
{
	plugin_register_config ("email", email_config, config_keys, config_keys_num);
	plugin_register_init ("email", email_init);
	plugin_register_read ("email", email_read);
	plugin_register_shutdown ("email", email_shutdown);
} /* void module_register */

/* vim: set sw=4 ts=4 tw=78 noexpandtab : */
