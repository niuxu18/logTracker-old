static void *Curl_polarssl_get_internals(struct ssl_connect_data *connssl,
                                         CURLINFO info UNUSED_PARAM)
{
  (void)info;
  return &BACKEND->ssl;
}