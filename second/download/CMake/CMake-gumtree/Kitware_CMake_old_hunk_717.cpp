  fflush(stdout);
  fflush(stderr);
  /* Sleep for 1 second.  */
#if defined(_WIN32)
  Sleep(1000);
#else
  sleep(1);
#endif
  fprintf(stdout, "Output on stdout after sleep.\n");
  fprintf(stderr, "Output on stderr after sleep.\n");
  fflush(stdout);
