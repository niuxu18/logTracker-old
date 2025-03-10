    default:
      break;
    }
  }

  if (print_timestamp) {
    time_t tt = CDTIME_T_TO_TIME_T(timestamp_time);
    localtime_r(&tt, &timestamp_tm);

    strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S",
             &timestamp_tm);
    timestamp_str[sizeof(timestamp_str) - 1] = '\0';
  }

  pthread_mutex_lock(&file_lock);

  if (log_file == NULL) {
    fh = stderr;
  } else if (strcasecmp(log_file, "stderr") == 0)
    fh = stderr;
  else if (strcasecmp(log_file, "stdout") == 0)
    fh = stdout;
  else {
    fh = fopen(log_file, "a");
    do_close = 1;
  }

  if (fh == NULL) {
    char errbuf[1024];
    fprintf(stderr, "logfile plugin: fopen (%s) failed: %s\n", log_file,
            sstrerror(errno, errbuf, sizeof(errbuf)));
  } else {
    if (print_timestamp)
      fprintf(fh, "[%s] %s%s\n", timestamp_str, level_str, msg);
    else
      fprintf(fh, "%s%s\n", level_str, msg);
