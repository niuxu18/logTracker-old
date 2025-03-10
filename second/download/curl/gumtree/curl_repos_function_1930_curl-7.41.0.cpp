CURLcode Curl_pp_sendf(struct pingpong *pp,
                       const char *fmt, ...)
{
  CURLcode result;
  va_list ap;
  va_start(ap, fmt);

  result = Curl_pp_vsendf(pp, fmt, ap);

  va_end(ap);

  return result;
}