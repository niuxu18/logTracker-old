      "    -D <port>      Destination port of the network packets.\n"
      "                   (Default: %s)\n"
      "    -h             Print usage information (this output).\n"
      "\n"
      "Copyright (C) 2010-2012  Florian Forster\n"
      "Licensed under the MIT license.\n",
      DEF_NUM_VALUES, DEF_NUM_HOSTS, DEF_NUM_PLUGINS,
      DEF_INTERVAL,
      NET_DEFAULT_V6_ADDR, NET_DEFAULT_PORT);
  exit (exit_status);
} /* }}} void exit_usage */

static void signal_handler (int signal) /* {{{ */
{
  loop = 0;
} /* }}} void signal_handler */

#if HAVE_CLOCK_GETTIME
static double dtime (void) /* {{{ */
{
  struct timespec ts = { 0 };

  if (clock_gettime (CLOCK_MONOTONIC, &ts) != 0)
    perror ("clock_gettime");

  return ((double) ts.tv_sec) + (((double) ts.tv_nsec) / 1e9);
} /* }}} double dtime */
#else
/* Work around for Mac OS X which doesn't have clock_gettime(2). *sigh* */
static double dtime (void) /* {{{ */
{
  struct timeval tv = { 0 };

  if (gettimeofday (&tv, /* timezone = */ NULL) != 0)
    perror ("gettimeofday");

  return ((double) tv.tv_sec) + (((double) tv.tv_usec) / 1e6);
} /* }}} double dtime */
#endif

static int compare_time (const void *v0, const void *v1) /* {{{ */
{
  const lcc_value_list_t *vl0 = v0;
  const lcc_value_list_t *vl1 = v1;

  if (vl0->time < vl1->time)
    return (-1);
  else if (vl0->time > vl1->time)
    return (1);
  else
    return (0);
} /* }}} int compare_time */

static int get_boundet_random (int min, int max) /* {{{ */
{
  int range;

  if (min >= max)
    return (-1);
  if (min == (max - 1))
    return (min);

  range = max - min;

  return (min + ((int) (((double) range) * ((double) random ()) / (((double) RAND_MAX) + 1.0))));
} /* }}} int get_boundet_random */

static lcc_value_list_t *create_value_list (void) /* {{{ */
{
  lcc_value_list_t *vl;
  int host_num;

  vl = calloc (1, sizeof (*vl));
  if (vl == NULL)
  {
    fprintf (stderr, "calloc failed.\n");
    return (NULL);
  }

  vl->values = calloc (/* nmemb = */ 1, sizeof (*vl->values));
  if (vl->values == NULL)
  {
    fprintf (stderr, "calloc failed.\n");
    free (vl);
    return (NULL);
  }

  vl->values_types = calloc (/* nmemb = */ 1, sizeof (*vl->values_types));
  if (vl->values_types == NULL)
  {
    fprintf (stderr, "calloc failed.\n");
    free (vl->values);
    free (vl);
    return (NULL);
  }

  vl->values_len = 1;

  host_num = get_boundet_random (0, conf_num_hosts);

  vl->interval = conf_interval;
  vl->time = 1.0 + dtime ()
    + (host_num % (1 + (int) vl->interval));

  if (get_boundet_random (0, 2) == 0)
    vl->values_types[0] = LCC_TYPE_GAUGE;
  else
    vl->values_types[0] = LCC_TYPE_DERIVE;

  snprintf (vl->identifier.host, sizeof (vl->identifier.host),
      "host%04i", host_num);
  snprintf (vl->identifier.plugin, sizeof (vl->identifier.plugin),
      "plugin%03i", get_boundet_random (0, conf_num_plugins));
  strncpy (vl->identifier.type,
      (vl->values_types[0] == LCC_TYPE_GAUGE) ? "gauge" : "derive",
      sizeof (vl->identifier.type));
  vl->identifier.type[sizeof (vl->identifier.type) - 1] = 0;
  snprintf (vl->identifier.type_instance, sizeof (vl->identifier.type_instance),
      "ti%li", random ());

  return (vl);
} /* }}} int create_value_list */

static void destroy_value_list (lcc_value_list_t *vl) /* {{{ */
{
  if (vl == NULL)
    return;

  free (vl->values);
  free (vl->values_types);
  free (vl);
} /* }}} void destroy_value_list */

static int send_value (lcc_value_list_t *vl) /* {{{ */
{
  int status;

  if (vl->values_types[0] == LCC_TYPE_GAUGE)
    vl->values[0].gauge = 100.0 * ((gauge_t) random ()) / (((gauge_t) RAND_MAX) + 1.0);
  else
    vl->values[0].derive += (derive_t) get_boundet_random (0, 100);

  status = lcc_network_values_send (net, vl);
  if (status != 0)
    fprintf (stderr, "lcc_network_values_send failed with status %i.\n", status);

  vl->time += vl->interval;

  return (0);
} /* }}} int send_value */

static int get_integer_opt (const char *str, int *ret_value) /* {{{ */
{
  char *endptr;
  int tmp;

  errno = 0;
  endptr = NULL;
  tmp = (int) strtol (str, &endptr, /* base = */ 0);
  if (errno != 0)
  {
    fprintf (stderr, "Unable to parse option as a number: \"%s\": %s\n",
        str, strerror (errno));
    exit (EXIT_FAILURE);
  }
  else if (endptr == str)
  {
    fprintf (stderr, "Unable to parse option as a number: \"%s\"\n", str);
    exit (EXIT_FAILURE);
  }
  else if (*endptr != 0)
  {
    fprintf (stderr, "Garbage after end of value: \"%s\"\n", str);
    exit (EXIT_FAILURE);
  }

  *ret_value = tmp;
  return (0);
} /* }}} int get_integer_opt */

static int get_double_opt (const char *str, double *ret_value) /* {{{ */
{
  char *endptr;
  double tmp;

  errno = 0;
  endptr = NULL;
  tmp = strtod (str, &endptr);
  if (errno != 0)
  {
    fprintf (stderr, "Unable to parse option as a number: \"%s\": %s\n",
        str, strerror (errno));
    exit (EXIT_FAILURE);
  }
  else if (endptr == str)
  {
    fprintf (stderr, "Unable to parse option as a number: \"%s\"\n", str);
    exit (EXIT_FAILURE);
  }
  else if (*endptr != 0)
  {
    fprintf (stderr, "Garbage after end of value: \"%s\"\n", str);
    exit (EXIT_FAILURE);
  }

  *ret_value = tmp;
  return (0);
} /* }}} int get_double_opt */

static int read_options (int argc, char **argv) /* {{{ */
{
  int opt;

  while ((opt = getopt (argc, argv, "n:H:p:i:d:D:h")) != -1)
  {
    switch (opt)
    {
      case 'n':
        get_integer_opt (optarg, &conf_num_values);
        break;

      case 'H':
        get_integer_opt (optarg, &conf_num_hosts);
        break;

      case 'p':
        get_integer_opt (optarg, &conf_num_plugins);
        break;

      case 'i':
        get_double_opt (optarg, &conf_interval);
        break;

      case 'd':
        conf_destination = optarg;
        break;

      case 'D':
        conf_service = optarg;
        break;

      case 'h':
        exit_usage (EXIT_SUCCESS);

      default:
        exit_usage (EXIT_FAILURE);
    } /* switch (opt) */
  } /* while (getopt) */

  return (0);
} /* }}} int read_options */

int main (int argc, char **argv) /* {{{ */
{
  double last_time;
  int values_sent = 0;

  read_options (argc, argv);

  sigint_action.sa_handler = signal_handler;
  sigaction (SIGINT, &sigint_action, /* old = */ NULL);

  sigterm_action.sa_handler = signal_handler;
  sigaction (SIGTERM, &sigterm_action, /* old = */ NULL);


  values_heap = c_heap_create (compare_time);
  if (values_heap == NULL)
  {
    fprintf (stderr, "c_heap_create failed.\n");
    exit (EXIT_FAILURE);
  }

  net = lcc_network_create ();
  if (net == NULL)
  {
    fprintf (stderr, "lcc_network_create failed.\n");
    exit (EXIT_FAILURE);
  }
  else
  {
    lcc_server_t *srv;

    srv = lcc_server_create (net, conf_destination, conf_service);
    if (srv == NULL)
    {
      fprintf (stderr, "lcc_server_create failed.\n");
      exit (EXIT_FAILURE);
    }

    lcc_server_set_ttl (srv, 42);
#if 0
    lcc_server_set_security_level (srv, ENCRYPT,
        "admin", "password1");
#endif
  }

  fprintf (stdout, "Creating %i values ... ", conf_num_values);
  fflush (stdout);
  for (int i = 0; i < conf_num_values; i++)
  {
    lcc_value_list_t *vl;

    vl = create_value_list ();
    if (vl == NULL)
    {
      fprintf (stderr, "create_value_list failed.\n");
      exit (EXIT_FAILURE);
    }

    c_heap_insert (values_heap, vl);
  }
  fprintf (stdout, "done\n");

  last_time = 0;
  while (loop)
  {
    lcc_value_list_t *vl = c_heap_get_root (values_heap);

    if (vl == NULL)
      break;

    if (vl->time != last_time)
    {
      printf ("%i values have been sent.\n", values_sent);

      /* Check if we need to sleep */
      double now = dtime ();

      while (now < vl->time)
      {
        /* 1 / 100 second */
        struct timespec ts = { 0, 10000000 };

        ts.tv_sec = (time_t) now;
        ts.tv_nsec = (long) ((now - ((double) ts.tv_sec)) * 1e9);

        nanosleep (&ts, /* remaining = */ NULL);
        now = dtime ();

        if (!loop)
          break;
      }
      last_time = vl->time;
    }

    send_value (vl);
    values_sent++;

    c_heap_insert (values_heap, vl);
  }

  fprintf (stdout, "Shutting down.\n");
  fflush (stdout);

  while (42)
  {
    lcc_value_list_t *vl = c_heap_get_root (values_heap);
    if (vl == NULL)
      break;
    destroy_value_list (vl);
  }
  c_heap_destroy (values_heap);

  lcc_network_destroy (net);
  exit (EXIT_SUCCESS);
} /* }}} int main */

/* vim: set sw=2 sts=2 et fdm=marker : */
