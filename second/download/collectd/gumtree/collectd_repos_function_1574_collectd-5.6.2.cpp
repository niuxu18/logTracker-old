static int wrr_config_node(oconfig_item_t *ci) /* {{{ */
{
  struct riemann_host *host = NULL;
  int status = 0;
  int i;
  oconfig_item_t *child;
  char callback_name[DATA_MAX_NAME_LEN];

  if ((host = calloc(1, sizeof(*host))) == NULL) {
    ERROR("write_riemann plugin: calloc failed.");
    return ENOMEM;
  }
  pthread_mutex_init(&host->lock, NULL);
  C_COMPLAIN_INIT(&host->init_complaint);
  host->reference_count = 1;
  host->node = NULL;
  host->port = 0;
  host->notifications = 1;
  host->check_thresholds = 0;
  host->store_rates = 1;
  host->always_append_ds = 0;
  host->batch_mode = 1;
  host->batch_max = RIEMANN_BATCH_MAX; /* typical MSS */
  host->batch_init = cdtime();
  host->batch_timeout = 0;
  host->ttl_factor = RIEMANN_TTL_FACTOR;
  host->client = NULL;
  host->client_type = RIEMANN_CLIENT_TCP;
  host->timeout.tv_sec = 0;
  host->timeout.tv_usec = 0;

  status = cf_util_get_string(ci, &host->name);
  if (status != 0) {
    WARNING("write_riemann plugin: Required host name is missing.");
    wrr_free(host);
    return -1;
  }

  for (i = 0; i < ci->children_num; i++) {
    /*
     * The code here could be simplified but makes room
     * for easy adding of new options later on.
     */
    child = &ci->children[i];
    status = 0;

    if (strcasecmp("Host", child->key) == 0) {
      status = cf_util_get_string(child, &host->node);
      if (status != 0)
        break;
    } else if (strcasecmp("Notifications", child->key) == 0) {
      status = cf_util_get_boolean(child, &host->notifications);
      if (status != 0)
        break;
    } else if (strcasecmp("EventServicePrefix", child->key) == 0) {
      status = cf_util_get_string(child, &host->event_service_prefix);
      if (status != 0)
        break;
    } else if (strcasecmp("CheckThresholds", child->key) == 0) {
      status = cf_util_get_boolean(child, &host->check_thresholds);
      if (status != 0)
        break;
    } else if (strcasecmp("Batch", child->key) == 0) {
      status = cf_util_get_boolean(child, &host->batch_mode);
      if (status != 0)
        break;
    } else if (strcasecmp("BatchMaxSize", child->key) == 0) {
      status = cf_util_get_int(child, &host->batch_max);
      if (status != 0)
        break;
    } else if (strcasecmp("BatchFlushTimeout", child->key) == 0) {
      status = cf_util_get_int(child, &host->batch_timeout);
      if (status != 0)
        break;
    } else if (strcasecmp("Timeout", child->key) == 0) {
#if RCC_VERSION_NUMBER >= 0x010800
      status = cf_util_get_int(child, (int *)&host->timeout.tv_sec);
      if (status != 0)
        break;
#else
      WARNING("write_riemann plugin: The Timeout option is not supported. "
              "Please upgrade the Riemann client to at least 1.8.0.");
#endif
    } else if (strcasecmp("Port", child->key) == 0) {
      host->port = cf_util_get_port_number(child);
      if (host->port == -1) {
        ERROR("write_riemann plugin: Invalid argument "
              "configured for the \"Port\" "
              "option.");
        break;
      }
    } else if (strcasecmp("Protocol", child->key) == 0) {
      char tmp[16];
      status = cf_util_get_string_buffer(child, tmp, sizeof(tmp));
      if (status != 0) {
        ERROR("write_riemann plugin: cf_util_get_"
              "string_buffer failed with "
              "status %i.",
              status);
        break;
      }

      if (strcasecmp("UDP", tmp) == 0)
        host->client_type = RIEMANN_CLIENT_UDP;
      else if (strcasecmp("TCP", tmp) == 0)
        host->client_type = RIEMANN_CLIENT_TCP;
      else if (strcasecmp("TLS", tmp) == 0)
        host->client_type = RIEMANN_CLIENT_TLS;
      else
        WARNING("write_riemann plugin: The value "
                "\"%s\" is not valid for the "
                "\"Protocol\" option. Use "
                "either \"UDP\", \"TCP\" or \"TLS\".",
                tmp);
    } else if (strcasecmp("TLSCAFile", child->key) == 0) {
      status = cf_util_get_string(child, &host->tls_ca_file);
      if (status != 0) {
        ERROR("write_riemann plugin: cf_util_get_"
              "string_buffer failed with "
              "status %i.",
              status);
        break;
      }
    } else if (strcasecmp("TLSCertFile", child->key) == 0) {
      status = cf_util_get_string(child, &host->tls_cert_file);
      if (status != 0) {
        ERROR("write_riemann plugin: cf_util_get_"
              "string_buffer failed with "
              "status %i.",
              status);
        break;
      }
    } else if (strcasecmp("TLSKeyFile", child->key) == 0) {
      status = cf_util_get_string(child, &host->tls_key_file);
      if (status != 0) {
        ERROR("write_riemann plugin: cf_util_get_"
              "string_buffer failed with "
              "status %i.",
              status);
        break;
      }
    } else if (strcasecmp("StoreRates", child->key) == 0) {
      status = cf_util_get_boolean(child, &host->store_rates);
      if (status != 0)
        break;
    } else if (strcasecmp("AlwaysAppendDS", child->key) == 0) {
      status = cf_util_get_boolean(child, &host->always_append_ds);
      if (status != 0)
        break;
    } else if (strcasecmp("TTLFactor", child->key) == 0) {
      double tmp = NAN;
      status = cf_util_get_double(child, &tmp);
      if (status != 0)
        break;
      if (tmp >= 2.0) {
        host->ttl_factor = tmp;
      } else if (tmp >= 1.0) {
        NOTICE("write_riemann plugin: The configured "
               "TTLFactor is very small "
               "(%.1f). A value of 2.0 or "
               "greater is recommended.",
               tmp);
        host->ttl_factor = tmp;
      } else if (tmp > 0.0) {
        WARNING("write_riemann plugin: The configured "
                "TTLFactor is too small to be "
                "useful (%.1f). I'll use it "
                "since the user knows best, "
                "but under protest.",
                tmp);
        host->ttl_factor = tmp;
      } else { /* zero, negative and NAN */
        ERROR("write_riemann plugin: The configured "
              "TTLFactor is invalid (%.1f).",
              tmp);
      }
    } else {
      WARNING("write_riemann plugin: ignoring unknown config "
              "option: \"%s\"",
              child->key);
    }
  }
  if (status != 0) {
    wrr_free(host);
    return status;
  }

  ssnprintf(callback_name, sizeof(callback_name), "write_riemann/%s",
            host->name);

  user_data_t ud = {.data = host, .free_func = wrr_free};

  pthread_mutex_lock(&host->lock);

  status = plugin_register_write(callback_name, wrr_write, &ud);

  if (host->client_type != RIEMANN_CLIENT_UDP && host->batch_mode) {
    ud.free_func = NULL;
    plugin_register_flush(callback_name, wrr_batch_flush, &ud);
  }
  if (status != 0)
    WARNING("write_riemann plugin: plugin_register_write (\"%s\") "
            "failed with status %i.",
            callback_name, status);
  else /* success */
    host->reference_count++;

  status = plugin_register_notification(callback_name, wrr_notification, &ud);
  if (status != 0)
    WARNING("write_riemann plugin: plugin_register_notification (\"%s\") "
            "failed with status %i.",
            callback_name, status);
  else /* success */
    host->reference_count++;

  if (host->reference_count <= 1) {
    /* Both callbacks failed => free memory.
     * We need to unlock here, because riemann_free() will lock.
     * This is not a race condition, because we're the only one
     * holding a reference. */
    pthread_mutex_unlock(&host->lock);
    wrr_free(host);
    return (-1);
  }

  host->reference_count--;
  pthread_mutex_unlock(&host->lock);

  return status;
}