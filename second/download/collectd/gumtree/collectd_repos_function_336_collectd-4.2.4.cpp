static int memcached_query_daemon (char *buffer, int buffer_size) /* {{{ */
{
	int fd;
	ssize_t status;
	int buffer_fill;

	const char *host;
	const char *port;

	struct addrinfo  ai_hints;
	struct addrinfo *ai_list, *ai_ptr;
	int              ai_return, i = 0;

	memset (&ai_hints, '\0', sizeof (ai_hints));
	ai_hints.ai_flags    = 0;
#ifdef AI_ADDRCONFIG
/*	ai_hints.ai_flags   |= AI_ADDRCONFIG; */
#endif
	ai_hints.ai_family   = AF_INET;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_protocol = 0;

	host = memcached_host;
	if (host == NULL) {
		host = MEMCACHED_DEF_HOST;
	}

	port = memcached_port;
	if (strlen (port) == 0) {
		port = MEMCACHED_DEF_PORT;
	}

	if ((ai_return = getaddrinfo (host, port, NULL, &ai_list)) != 0) {
		char errbuf[1024];
		ERROR ("memcached: getaddrinfo (%s, %s): %s",
				host, port,
				(ai_return == EAI_SYSTEM)
				? sstrerror (errno, errbuf, sizeof (errbuf))
				: gai_strerror (ai_return));
		return -1;
	}

	fd = -1;
	for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
		/* create our socket descriptor */
		if ((fd = socket (ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol)) < 0) {
			char errbuf[1024];
			ERROR ("memcached: socket: %s", sstrerror (errno, errbuf, sizeof (errbuf)));
			continue;
		}

		/* connect to the memcached daemon */
		if (connect (fd, (struct sockaddr *) ai_ptr->ai_addr, ai_ptr->ai_addrlen)) {
			shutdown(fd, SHUT_RDWR);
			close(fd);
			fd = -1;
			continue;
		}

		/* A socket could be opened and connecting succeeded. We're
		 * done. */
		break;
	}

	freeaddrinfo (ai_list);

	if (fd < 0) {
		ERROR ("memcached: Could not connect to daemon.");
		return -1;
	}

	if (send(fd, "stats\r\n", sizeof("stats\r\n") - 1, MSG_DONTWAIT) != (sizeof("stats\r\n") - 1)) {
		ERROR ("memcached: Could not send command to the memcached daemon.");
		return -1;
	}

	{
		struct pollfd p;
		int n;

		p.fd = fd;
		p.events = POLLIN|POLLERR|POLLHUP;
		p.revents = 0;

		n = poll(&p, 1, 3);

		if (n <= 0) {
			ERROR ("memcached: poll() failed or timed out");
			return -1;
		}
	}

	/* receive data from the memcached daemon */
	memset (buffer, '\0', buffer_size);

	buffer_fill = 0;
	while ((status = recv (fd, buffer + buffer_fill, buffer_size - buffer_fill, MSG_DONTWAIT)) != 0) {
		if (i > MEMCACHED_RETRY_COUNT) {
			ERROR("recv() timed out");
			break;
		}
		i++;

		if (status == -1) {
			char errbuf[1024];

			if (errno == EAGAIN) {
				continue;
			}

			ERROR ("memcached: Error reading from socket: %s",
					sstrerror (errno, errbuf, sizeof (errbuf)));
			shutdown(fd, SHUT_RDWR);
			close (fd);
			return -1;
		}
		buffer_fill += status;

		if (buffer_fill > 3 && buffer[buffer_fill-5] == 'E' && buffer[buffer_fill-4] == 'N' && buffer[buffer_fill-3] == 'D') {
			/* we got all the data */
			break;
		}
	}

	if (buffer_fill >= buffer_size) {
		buffer[buffer_size - 1] = '\0';
		WARNING ("memcached: Message from memcached has been truncated.");
	} else if (buffer_fill == 0) {
		WARNING ("memcached: Peer has unexpectedly shut down the socket. "
				"Buffer: `%s'", buffer);
		shutdown(fd, SHUT_RDWR);
		close(fd);
		return -1;
	}

	shutdown(fd, SHUT_RDWR);
	close(fd);
	return 0;
}