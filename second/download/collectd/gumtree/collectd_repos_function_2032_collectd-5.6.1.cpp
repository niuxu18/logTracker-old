static int ctail_config_add_match_dstype (ctail_config_match_t *cm,
    oconfig_item_t *ci)
{
  if ((ci->values_num != 1) || (ci->values[0].type != OCONFIG_TYPE_STRING))
  {
    WARNING ("tail plugin: `DSType' needs exactly one string argument.");
    return (-1);
  }

  if (strncasecmp ("Gauge", ci->values[0].value.string, strlen ("Gauge")) == 0)
  {
    cm->flags = UTILS_MATCH_DS_TYPE_GAUGE;
    if (strcasecmp ("GaugeAverage", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_GAUGE_AVERAGE;
    else if (strcasecmp ("GaugeMin", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_GAUGE_MIN;
    else if (strcasecmp ("GaugeMax", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_GAUGE_MAX;
    else if (strcasecmp ("GaugeLast", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_GAUGE_LAST;
    else if (strcasecmp ("GaugeInc", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_GAUGE_INC;
    else if (strcasecmp ("GaugeAdd", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_GAUGE_ADD;
    else
      cm->flags = 0;
  }
  else if (strncasecmp ("Counter", ci->values[0].value.string, strlen ("Counter")) == 0)
  {
    cm->flags = UTILS_MATCH_DS_TYPE_COUNTER;
    if (strcasecmp ("CounterSet", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_COUNTER_SET;
    else if (strcasecmp ("CounterAdd", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_COUNTER_ADD;
    else if (strcasecmp ("CounterInc", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_COUNTER_INC;
    else
      cm->flags = 0;
  }
  else if (strncasecmp ("Derive", ci->values[0].value.string, strlen ("Derive")) == 0)
  {
    cm->flags = UTILS_MATCH_DS_TYPE_DERIVE;
    if (strcasecmp ("DeriveSet", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_DERIVE_SET;
    else if (strcasecmp ("DeriveAdd", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_DERIVE_ADD;
    else if (strcasecmp ("DeriveInc", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_DERIVE_INC;
    else
      cm->flags = 0;
  }
  else if (strncasecmp ("Absolute", ci->values[0].value.string, strlen ("Absolute")) == 0)
  {
    cm->flags = UTILS_MATCH_DS_TYPE_ABSOLUTE;
    if (strcasecmp ("AbsoluteSet", ci->values[0].value.string) == 0)
      cm->flags |= UTILS_MATCH_CF_ABSOLUTE_SET;
    else
      cm->flags = 0;
  }
  else
  {
    cm->flags = 0;
  }

  if (cm->flags == 0)
  {
    WARNING ("tail plugin: `%s' is not a valid argument to `DSType'.",
	ci->values[0].value.string);
    return (-1);
  }

  return (0);
}