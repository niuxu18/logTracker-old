static int uptime_read(void) {
  gauge_t uptime;
  time_t elapsed;

  /* calculate the amount of time elapsed since boot, AKA uptime */
  elapsed = time(NULL) - boottime;

  uptime = (gauge_t)elapsed;

  uptime_submit(uptime);

  return (0);
}