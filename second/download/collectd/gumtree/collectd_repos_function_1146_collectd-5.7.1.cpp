static int submit_two_derive(const char *host,
                             const char *plugin_inst, /* {{{ */
                             const char *type, const char *type_inst,
                             derive_t val0, derive_t val1, cdtime_t timestamp,
                             cdtime_t interval) {
  value_t values[] = {
      {.derive = val0}, {.derive = val1},
  };

  return (submit_values(host, plugin_inst, type, type_inst, values,
                        STATIC_ARRAY_SIZE(values), timestamp, interval));
}