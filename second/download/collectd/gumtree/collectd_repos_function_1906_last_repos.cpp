static int ut_report_state(const data_set_t *ds, const value_list_t *vl,
                           const threshold_t *th, const gauge_t *values,
                           int ds_index, int state) { /* {{{ */
  int state_old;
  notification_t n;

  char *buf;
  size_t bufsize;

  int status;

  /* Check if hits matched */
  if ((th->hits != 0)) {
    int hits = uc_get_hits(ds, vl);
    /* STATE_OKAY resets hits unless PERSIST_OK flag is set. Hits resets if
     * threshold is hit. */
    if (((state == STATE_OKAY) && ((th->flags & UT_FLAG_PERSIST_OK) == 0)) ||
        (hits > th->hits)) {
      DEBUG("ut_report_state: reset uc_get_hits = 0");
      uc_set_hits(ds, vl, 0); /* reset hit counter and notify */
    } else {
      DEBUG("ut_report_state: th->hits = %d, uc_get_hits = %d", th->hits,
            uc_get_hits(ds, vl));
      (void)uc_inc_hits(ds, vl, 1); /* increase hit counter */
      return 0;
    }
  } /* end check hits */

  state_old = uc_get_state(ds, vl);

  /* If the state didn't change, report if `persistent' is specified. If the
   * state is `okay', then only report if `persist_ok` flag is set. */
  if (state == state_old) {
    if ((th->flags & UT_FLAG_PERSIST) == 0)
      return 0;
    else if ((state == STATE_OKAY) && ((th->flags & UT_FLAG_PERSIST_OK) == 0))
      return 0;
  }

  if (state != state_old)
    uc_set_state(ds, vl, state);

  NOTIFICATION_INIT_VL(&n, vl);

  buf = n.message;
  bufsize = sizeof(n.message);

  if (state == STATE_OKAY)
    n.severity = NOTIF_OKAY;
  else if (state == STATE_WARNING)
    n.severity = NOTIF_WARNING;
  else
    n.severity = NOTIF_FAILURE;

  n.time = vl->time;

  status = snprintf(buf, bufsize, "Host %s, plugin %s", vl->host, vl->plugin);
  buf += status;
  bufsize -= status;

  if (vl->plugin_instance[0] != '\0') {
    status = snprintf(buf, bufsize, " (instance %s)", vl->plugin_instance);
    buf += status;
    bufsize -= status;
  }

  status = snprintf(buf, bufsize, " type %s", vl->type);
  buf += status;
  bufsize -= status;

  if (vl->type_instance[0] != '\0') {
    status = snprintf(buf, bufsize, " (instance %s)", vl->type_instance);
    buf += status;
    bufsize -= status;
  }

  plugin_notification_meta_add_string(&n, "DataSource", ds->ds[ds_index].name);
  plugin_notification_meta_add_double(&n, "CurrentValue", values[ds_index]);
  plugin_notification_meta_add_double(&n, "WarningMin", th->warning_min);
  plugin_notification_meta_add_double(&n, "WarningMax", th->warning_max);
  plugin_notification_meta_add_double(&n, "FailureMin", th->failure_min);
  plugin_notification_meta_add_double(&n, "FailureMax", th->failure_max);

  /* Send an okay notification */
  if (state == STATE_OKAY) {
    if (state_old == STATE_MISSING)
      snprintf(buf, bufsize, ": Value is no longer missing.");
    else
      snprintf(buf, bufsize, ": All data sources are within range again. "
                             "Current value of \"%s\" is %f.",
               ds->ds[ds_index].name, values[ds_index]);
  } else {
    double min;
    double max;

    min = (state == STATE_ERROR) ? th->failure_min : th->warning_min;
    max = (state == STATE_ERROR) ? th->failure_max : th->warning_max;

    if (th->flags & UT_FLAG_INVERT) {
      if (!isnan(min) && !isnan(max)) {
        snprintf(buf, bufsize,
                 ": Data source \"%s\" is currently "
                 "%f. That is within the %s region of %f%s and %f%s.",
                 ds->ds[ds_index].name, values[ds_index],
                 (state == STATE_ERROR) ? "failure" : "warning", min,
                 ((th->flags & UT_FLAG_PERCENTAGE) != 0) ? "%" : "", max,
                 ((th->flags & UT_FLAG_PERCENTAGE) != 0) ? "%" : "");
      } else {
        snprintf(buf, bufsize, ": Data source \"%s\" is currently "
                               "%f. That is %s the %s threshold of %f%s.",
                 ds->ds[ds_index].name, values[ds_index],
                 isnan(min) ? "below" : "above",
                 (state == STATE_ERROR) ? "failure" : "warning",
                 isnan(min) ? max : min,
                 ((th->flags & UT_FLAG_PERCENTAGE) != 0) ? "%" : "");
      }
    } else if (th->flags & UT_FLAG_PERCENTAGE) {
      gauge_t value;
      gauge_t sum;

      sum = 0.0;
      for (size_t i = 0; i < vl->values_len; i++) {
        if (isnan(values[i]))
          continue;

        sum += values[i];
      }

      if (sum == 0.0)
        value = NAN;
      else
        value = 100.0 * values[ds_index] / sum;

      snprintf(buf, bufsize,
               ": Data source \"%s\" is currently "
               "%g (%.2f%%). That is %s the %s threshold of %.2f%%.",
               ds->ds[ds_index].name, values[ds_index], value,
               (value < min) ? "below" : "above",
               (state == STATE_ERROR) ? "failure" : "warning",
               (value < min) ? min : max);
    } else /* is not inverted */
    {
      snprintf(buf, bufsize, ": Data source \"%s\" is currently "
                             "%f. That is %s the %s threshold of %f.",
               ds->ds[ds_index].name, values[ds_index],
               (values[ds_index] < min) ? "below" : "above",
               (state == STATE_ERROR) ? "failure" : "warning",
               (values[ds_index] < min) ? min : max);
    }
  }

  plugin_dispatch_notification(&n);

  plugin_notification_meta_free(n.meta);
  return 0;
}