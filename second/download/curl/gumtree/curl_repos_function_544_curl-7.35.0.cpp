static CURLcode unit_setup( void )
{
  data = curl_easy_init();
  if (!data)
    return CURLE_OUT_OF_MEMORY;

  hp = Curl_mk_dnscache();
  if(!hp) {
    curl_easy_cleanup(data);
    curl_global_cleanup();
    return CURLE_OUT_OF_MEMORY;
  }
  return CURLE_OK;
}