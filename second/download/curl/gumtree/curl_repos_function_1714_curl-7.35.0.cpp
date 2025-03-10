time_t curl_getdate(const char *p, const time_t *now)
{
  time_t parsed;
  int rc = parsedate(p, &parsed);
  (void)now; /* legacy argument from the past that we ignore */

  switch(rc) {
  case PARSEDATE_OK:
  case PARSEDATE_LATER:
  case PARSEDATE_SOONER:
    return parsed;
  }
  /* everything else is fail */
  return -1;
}