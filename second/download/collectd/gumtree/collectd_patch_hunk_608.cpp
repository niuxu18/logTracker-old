 
 #include <string.h>
 
 #include <syslog.h>
 
 #include <sys/resource.h>
+#include <sys/stat.h>
 #include <sys/time.h>
 #include <sys/types.h>
-#include <sys/stat.h>
 #include <sys/wait.h>
 
 #include <time.h>
 
 #include <unistd.h>
 
 #ifndef PREFIX
-# define PREFIX "/opt/" PACKAGE_NAME
+#define PREFIX "/opt/" PACKAGE_NAME
 #endif
 
 #ifndef LOCALSTATEDIR
-# define LOCALSTATEDIR PREFIX "/var"
+#define LOCALSTATEDIR PREFIX "/var"
 #endif
 
 #ifndef COLLECTDMON_PIDFILE
-# define COLLECTDMON_PIDFILE LOCALSTATEDIR"/run/collectdmon.pid"
+#define COLLECTDMON_PIDFILE LOCALSTATEDIR "/run/collectdmon.pid"
 #endif /* ! COLLECTDMON_PIDFILE */
 
 #ifndef WCOREDUMP
-# define WCOREDUMP(s) 0
+#define WCOREDUMP(s) 0
 #endif /* ! WCOREDUMP */
 
-static int loop    = 0;
+static int loop = 0;
 static int restart = 0;
 
-static const char *pidfile      = NULL;
-static pid_t       collectd_pid = 0;
+static const char *pidfile = NULL;
+static pid_t collectd_pid = 0;
+
+__attribute__((noreturn)) static void exit_usage(const char *name) {
+  printf("Usage: %s <options> [-- <collectd options>]\n"
 
-__attribute__((noreturn))
-static void exit_usage (const char *name)
-{
-	printf ("Usage: %s <options> [-- <collectd options>]\n"
-
-			"\nAvailable options:\n"
-			"  -h         Display this help and exit.\n"
-			"  -c <path>  Path to the collectd binary.\n"
-			"  -P <file>  PID-file.\n"
-
-			"\nFor <collectd options> see collectd.conf(5).\n"
-
-			"\n"PACKAGE_NAME" "PACKAGE_VERSION", http://collectd.org/\n"
-			"by Florian octo Forster <octo@collectd.org>\n"
-			"for contributions see `AUTHORS'\n", name);
-	exit (0);
+         "\nAvailable options:\n"
+         "  -h         Display this help and exit.\n"
+         "  -c <path>  Path to the collectd binary.\n"
+         "  -P <file>  PID-file.\n"
+
+         "\nFor <collectd options> see collectd.conf(5).\n"
+
+         "\n" PACKAGE_NAME " " PACKAGE_VERSION ", http://collectd.org/\n"
+         "by Florian octo Forster <octo@collectd.org>\n"
+         "for contributions see `AUTHORS'\n",
+         name);
+  exit(0);
 } /* exit_usage */
 
-static int pidfile_create (void)
-{
-	FILE *file = NULL;
-
-	if (NULL == pidfile)
-		pidfile = COLLECTDMON_PIDFILE;
-
-	if (NULL == (file = fopen (pidfile, "w"))) {
-		syslog (LOG_ERR, "Error: couldn't open PID-file (%s) for writing: %s",
-				pidfile, strerror (errno));
-		return -1;
-	}
-
-	fprintf (file, "%i\n", (int)getpid ());
-	fclose (file);
-	return 0;
+static int pidfile_create(void) {
+  FILE *file = NULL;
+
+  if (NULL == pidfile)
+    pidfile = COLLECTDMON_PIDFILE;
+
+  if (NULL == (file = fopen(pidfile, "w"))) {
+    syslog(LOG_ERR, "Error: couldn't open PID-file (%s) for writing: %s",
+           pidfile, strerror(errno));
+    return -1;
+  }
+
+  fprintf(file, "%i\n", (int)getpid());
+  fclose(file);
+  return 0;
 } /* pidfile_create */
 
-static int pidfile_delete (void)
-{
-	assert (NULL != pidfile);
-
-	if (0 != unlink (pidfile)) {
-		syslog (LOG_ERR, "Error: couldn't delete PID-file (%s): %s",
-				pidfile, strerror (errno));
-		return -1;
-	}
-	return 0;
-} /* pidfile_remove */
+static int pidfile_delete(void) {
+  assert(NULL != pidfile);
 
-static int daemonize (void)
-{
-	struct rlimit rl;
-	int dev_null;
-
-	pid_t pid = 0;
-	int   i   = 0;
-
-	if (0 != chdir ("/")) {
-		fprintf (stderr, "Error: chdir() failed: %s\n", strerror (errno));
-		return -1;
-	}
-
-	if (0 != getrlimit (RLIMIT_NOFILE, &rl)) {
-		fprintf (stderr, "Error: getrlimit() failed: %s\n", strerror (errno));
-		return -1;
-	}
-
-	if (0 > (pid = fork ())) {
-		fprintf (stderr, "Error: fork() failed: %s\n", strerror (errno));
-		return -1;
-	}
-	else if (pid != 0) {
-		exit (0);
-	}
-
-	if (0 != pidfile_create ())
-		return -1;
-
-	setsid ();
-
-	if (RLIM_INFINITY == rl.rlim_max)
-		rl.rlim_max = 1024;
-
-	for (i = 0; i < (int)rl.rlim_max; ++i)
-		close (i);
-
-	dev_null = open ("/dev/null", O_RDWR);
-	if (dev_null == -1) {
-		syslog (LOG_ERR, "Error: couldn't open /dev/null: %s", strerror (errno));
-		return -1;
-	}
-
-	if (dup2 (dev_null, STDIN_FILENO) == -1) {
-		close (dev_null);
-		syslog (LOG_ERR, "Error: couldn't connect STDIN to /dev/null: %s", strerror (errno));
-		return -1;
-	}
-
-	if (dup2 (dev_null, STDOUT_FILENO) == -1) {
-		close (dev_null);
-		syslog (LOG_ERR, "Error: couldn't connect STDOUT to /dev/null: %s", strerror (errno));
-		return -1;
-	}
-
-	if (dup2 (dev_null, STDERR_FILENO) == -1) {
-		close (dev_null);
-		syslog (LOG_ERR, "Error: couldn't connect STDERR to /dev/null: %s", strerror (errno));
-		return -1;
-	}
+  if (0 != unlink(pidfile)) {
+    syslog(LOG_ERR, "Error: couldn't delete PID-file (%s): %s", pidfile,
+           strerror(errno));
+    return -1;
+  }
+  return 0;
+} /* pidfile_remove */
 
-	if ((dev_null != STDIN_FILENO) && (dev_null != STDOUT_FILENO) && (dev_null != STDERR_FILENO))
-		close (dev_null);
+static int daemonize(void) {
+  struct rlimit rl;
+  int dev_null;
+
+  pid_t pid = 0;
+  int i = 0;
+
+  if (0 != chdir("/")) {
+    fprintf(stderr, "Error: chdir() failed: %s\n", strerror(errno));
+    return -1;
+  }
+
+  if (0 != getrlimit(RLIMIT_NOFILE, &rl)) {
+    fprintf(stderr, "Error: getrlimit() failed: %s\n", strerror(errno));
+    return -1;
+  }
+
+  if (0 > (pid = fork())) {
+    fprintf(stderr, "Error: fork() failed: %s\n", strerror(errno));
+    return -1;
+  } else if (pid != 0) {
+    exit(0);
+  }
+
+  if (0 != pidfile_create())
+    return -1;
+
+  setsid();
+
+  if (RLIM_INFINITY == rl.rlim_max)
+    rl.rlim_max = 1024;
+
+  for (i = 0; i < (int)rl.rlim_max; ++i)
+    close(i);
+
+  dev_null = open("/dev/null", O_RDWR);
+  if (dev_null == -1) {
+    syslog(LOG_ERR, "Error: couldn't open /dev/null: %s", strerror(errno));
+    return -1;
+  }
+
+  if (dup2(dev_null, STDIN_FILENO) == -1) {
+    close(dev_null);
+    syslog(LOG_ERR, "Error: couldn't connect STDIN to /dev/null: %s",
+           strerror(errno));
+    return -1;
+  }
+
+  if (dup2(dev_null, STDOUT_FILENO) == -1) {
+    close(dev_null);
+    syslog(LOG_ERR, "Error: couldn't connect STDOUT to /dev/null: %s",
+           strerror(errno));
+    return -1;
+  }
+
+  if (dup2(dev_null, STDERR_FILENO) == -1) {
+    close(dev_null);
+    syslog(LOG_ERR, "Error: couldn't connect STDERR to /dev/null: %s",
+           strerror(errno));
+    return -1;
+  }
+
+  if ((dev_null != STDIN_FILENO) && (dev_null != STDOUT_FILENO) &&
+      (dev_null != STDERR_FILENO))
+    close(dev_null);
 
-	return 0;
+  return 0;
 } /* daemonize */
 
-static int collectd_start (char **argv)
-{
-	pid_t pid = 0;
-
-	if (0 > (pid = fork ())) {
-		syslog (LOG_ERR, "Error: fork() failed: %s", strerror (errno));
-		return -1;
-	}
-	else if (pid != 0) {
-		collectd_pid = pid;
-		return 0;
-	}
-
-	execvp (argv[0], argv);
-	syslog (LOG_ERR, "Error: execvp(%s) failed: %s",
-			argv[0], strerror (errno));
-	exit (-1);
+static int collectd_start(char **argv) {
+  pid_t pid = 0;
+
+  if (0 > (pid = fork())) {
+    syslog(LOG_ERR, "Error: fork() failed: %s", strerror(errno));
+    return -1;
+  } else if (pid != 0) {
+    collectd_pid = pid;
+    return 0;
+  }
+
+  execvp(argv[0], argv);
+  syslog(LOG_ERR, "Error: execvp(%s) failed: %s", argv[0], strerror(errno));
+  exit(-1);
 } /* collectd_start */
 
-static int collectd_stop (void)
-{
-	if (0 == collectd_pid)
-		return 0;
-
-	if (0 != kill (collectd_pid, SIGTERM)) {
-		syslog (LOG_ERR, "Error: kill() failed: %s", strerror (errno));
-		return -1;
-	}
-	return 0;
+static int collectd_stop(void) {
+  if (0 == collectd_pid)
+    return 0;
+
+  if (0 != kill(collectd_pid, SIGTERM)) {
+    syslog(LOG_ERR, "Error: kill() failed: %s", strerror(errno));
+    return -1;
+  }
+  return 0;
 } /* collectd_stop */
 
-static void sig_int_term_handler (int __attribute__((unused)) signo)
-{
-	++loop;
-	return;
+static void sig_int_term_handler(int __attribute__((unused)) signo) {
+  ++loop;
+  return;
 } /* sig_int_term_handler */
 
-static void sig_hup_handler (int __attribute__((unused)) signo)
-{
-	++restart;
-	return;
+static void sig_hup_handler(int __attribute__((unused)) signo) {
+  ++restart;
+  return;
 } /* sig_hup_handler */
 
-static void log_status (int status)
-{
-	if (WIFEXITED (status)) {
-		if (0 == WEXITSTATUS (status))
-			syslog (LOG_INFO, "Info: collectd terminated with exit status %i",
-					WEXITSTATUS (status));
-		else
-			syslog (LOG_WARNING,
-					"Warning: collectd terminated with exit status %i",
-					WEXITSTATUS (status));
-	}
-	else if (WIFSIGNALED (status)) {
-		syslog (LOG_WARNING, "Warning: collectd was terminated by signal %i%s",
-				WTERMSIG (status), WCOREDUMP (status) ? " (core dumped)" : "");
-	}
-	return;
+static void log_status(int status) {
+  if (WIFEXITED(status)) {
+    if (0 == WEXITSTATUS(status))
+      syslog(LOG_INFO, "Info: collectd terminated with exit status %i",
+             WEXITSTATUS(status));
+    else
+      syslog(LOG_WARNING, "Warning: collectd terminated with exit status %i",
+             WEXITSTATUS(status));
+  } else if (WIFSIGNALED(status)) {
+    syslog(LOG_WARNING, "Warning: collectd was terminated by signal %i%s",
+           WTERMSIG(status), WCOREDUMP(status) ? " (core dumped)" : "");
+  }
+  return;
 } /* log_status */
 
-static void check_respawn (void)
-{
-	time_t t = time (NULL);
-
-	static time_t timestamp = 0;
-	static int    counter   = 0;
-
-	if ((t - 120) < timestamp)
-		++counter;
-	else {
-		timestamp = t;
-		counter   = 0;
-	}
-
-	if (10 < counter) {
-		unsigned int time_left = 300;
-
-		syslog (LOG_ERR, "Error: collectd is respawning too fast - "
-				"disabled for %i seconds", time_left);
-
-		while ((0 < (time_left = sleep (time_left))) && (0 == loop));
-	}
-	return;
+static void check_respawn(void) {
+  time_t t = time(NULL);
+
+  static time_t timestamp = 0;
+  static int counter = 0;
+
+  if ((t - 120) < timestamp)
+    ++counter;
+  else {
+    timestamp = t;
+    counter = 0;
+  }
+
+  if (10 < counter) {
+    unsigned int time_left = 300;
+
+    syslog(LOG_ERR, "Error: collectd is respawning too fast - "
+                    "disabled for %i seconds",
+           time_left);
+
+    while ((0 < (time_left = sleep(time_left))) && (0 == loop))
+      ;
+  }
+  return;
 } /* check_respawn */
 
-int main (int argc, char **argv)
-{
-	int    collectd_argc = 0;
-	char  *collectd      = NULL;
-	char **collectd_argv = NULL;
-
-	struct sigaction sa;
-
-	int i = 0;
-
-	/* parse command line options */
-	while (42) {
-		int c = getopt (argc, argv, "hc:P:");
-
-		if (-1 == c)
-			break;
-
-		switch (c) {
-			case 'c':
-				collectd = optarg;
-				break;
-			case 'P':
-				pidfile = optarg;
-				break;
-			case 'h':
-			default:
-				exit_usage (argv[0]);
-		}
-	}
-
-	for (i = optind; i < argc; ++i)
-		if (0 == strcmp (argv[i], "-f"))
-			break;
-
-	/* i < argc => -f already present */
-	collectd_argc = 1 + argc - optind + ((i < argc) ? 0 : 1);
-	collectd_argv = (char **)calloc (collectd_argc + 1, sizeof (char *));
-
-	if (NULL == collectd_argv) {
-		fprintf (stderr, "Out of memory.");
-		return 3;
-	}
-
-	collectd_argv[0] = (NULL == collectd) ? "collectd" : collectd;
-
-	if (i == argc)
-		collectd_argv[collectd_argc - 1] = "-f";
-
-	for (i = optind; i < argc; ++i)
-		collectd_argv[i - optind + 1] = argv[i];
-
-	collectd_argv[collectd_argc] = NULL;
-
-	openlog ("collectdmon", LOG_CONS | LOG_PID, LOG_DAEMON);
-
-	if (-1 == daemonize ())
-	{
-		free (collectd_argv);
-		return 1;
-	}
-
-	sa.sa_handler = sig_int_term_handler;
-	sa.sa_flags   = 0;
-	sigemptyset (&sa.sa_mask);
-
-	if (0 != sigaction (SIGINT, &sa, NULL)) {
-		syslog (LOG_ERR, "Error: sigaction() failed: %s", strerror (errno));
-		free (collectd_argv);
-		return 1;
-	}
-
-	if (0 != sigaction (SIGTERM, &sa, NULL)) {
-		syslog (LOG_ERR, "Error: sigaction() failed: %s", strerror (errno));
-		free (collectd_argv);
-		return 1;
-	}
-
-	sa.sa_handler = sig_hup_handler;
-
-	if (0 != sigaction (SIGHUP, &sa, NULL)) {
-		syslog (LOG_ERR, "Error: sigaction() failed: %s", strerror (errno));
-		free (collectd_argv);
-		return 1;
-	}
-
-	while (0 == loop) {
-		int status = 0;
-
-		if (0 != collectd_start (collectd_argv)) {
-			syslog (LOG_ERR, "Error: failed to start collectd.");
-			break;
-		}
-
-		assert (0 < collectd_pid);
-		while ((collectd_pid != waitpid (collectd_pid, &status, 0))
-				&& (EINTR == errno))
-			if ((0 != loop) || (0 != restart))
-				collectd_stop ();
-
-		collectd_pid = 0;
-
-		log_status (status);
-		check_respawn ();
-
-		if (0 != restart) {
-			syslog (LOG_INFO, "Info: restarting collectd");
-			restart = 0;
-		}
-		else if (0 == loop)
-			syslog (LOG_WARNING, "Warning: restarting collectd");
-	}
+int main(int argc, char **argv) {
+  int collectd_argc = 0;
+  char *collectd = NULL;
+  char **collectd_argv = NULL;
+
+  struct sigaction sa;
+
+  int i = 0;
+
+  /* parse command line options */
+  while (42) {
+    int c = getopt(argc, argv, "hc:P:");
+
+    if (-1 == c)
+      break;
+
+    switch (c) {
+    case 'c':
+      collectd = optarg;
+      break;
+    case 'P':
+      pidfile = optarg;
+      break;
+    case 'h':
+    default:
+      exit_usage(argv[0]);
+    }
+  }
+
+  for (i = optind; i < argc; ++i)
+    if (0 == strcmp(argv[i], "-f"))
+      break;
+
+  /* i < argc => -f already present */
+  collectd_argc = 1 + argc - optind + ((i < argc) ? 0 : 1);
+  collectd_argv = (char **)calloc(collectd_argc + 1, sizeof(char *));
+
+  if (NULL == collectd_argv) {
+    fprintf(stderr, "Out of memory.");
+    return 3;
+  }
+
+  collectd_argv[0] = (NULL == collectd) ? "collectd" : collectd;
+
+  if (i == argc)
+    collectd_argv[collectd_argc - 1] = "-f";
+
+  for (i = optind; i < argc; ++i)
+    collectd_argv[i - optind + 1] = argv[i];
+
+  collectd_argv[collectd_argc] = NULL;
+
+  openlog("collectdmon", LOG_CONS | LOG_PID, LOG_DAEMON);
+
+  if (-1 == daemonize()) {
+    free(collectd_argv);
+    return 1;
+  }
+
+  sa.sa_handler = sig_int_term_handler;
+  sa.sa_flags = 0;
+  sigemptyset(&sa.sa_mask);
+
+  if (0 != sigaction(SIGINT, &sa, NULL)) {
+    syslog(LOG_ERR, "Error: sigaction() failed: %s", strerror(errno));
+    free(collectd_argv);
+    return 1;
+  }
+
+  if (0 != sigaction(SIGTERM, &sa, NULL)) {
+    syslog(LOG_ERR, "Error: sigaction() failed: %s", strerror(errno));
+    free(collectd_argv);
+    return 1;
+  }
+
+  sa.sa_handler = sig_hup_handler;
+
+  if (0 != sigaction(SIGHUP, &sa, NULL)) {
+    syslog(LOG_ERR, "Error: sigaction() failed: %s", strerror(errno));
+    free(collectd_argv);
+    return 1;
+  }
+
+  while (0 == loop) {
+    int status = 0;
+
+    if (0 != collectd_start(collectd_argv)) {
+      syslog(LOG_ERR, "Error: failed to start collectd.");
+      break;
+    }
+
+    assert(0 < collectd_pid);
+    while ((collectd_pid != waitpid(collectd_pid, &status, 0)) &&
+           (EINTR == errno))
+      if ((0 != loop) || (0 != restart))
+        collectd_stop();
+
+    collectd_pid = 0;
+
+    log_status(status);
+    check_respawn();
+
+    if (0 != restart) {
+      syslog(LOG_INFO, "Info: restarting collectd");
+      restart = 0;
+    } else if (0 == loop)
+      syslog(LOG_WARNING, "Warning: restarting collectd");
+  }
 
-	syslog (LOG_INFO, "Info: shutting down collectdmon");
+  syslog(LOG_INFO, "Info: shutting down collectdmon");
 
-	pidfile_delete ();
-	closelog ();
+  pidfile_delete();
+  closelog();
 
-	free (collectd_argv);
-	return 0;
+  free(collectd_argv);
+  return 0;
 } /* main */
 
 /* vim: set sw=4 ts=4 tw=78 noexpandtab : */
-
