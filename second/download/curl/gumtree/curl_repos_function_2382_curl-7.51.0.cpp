void Curl_ssl_sessionid_lock(struct connectdata *conn)
{
  if(SSLSESSION_SHARED(conn->data))
    Curl_share_lock(conn->data,
                    CURL_LOCK_DATA_SSL_SESSION, CURL_LOCK_ACCESS_SINGLE);
}