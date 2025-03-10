static CURLcode read_data(struct connectdata *conn,
                          curl_socket_t fd,
                          struct krb5buffer *buf)
{
  int len;
  void* tmp;
  CURLcode ret;

  ret = socket_read(fd, &len, sizeof(len));
  if(ret != CURLE_OK)
    return ret;

  len = ntohl(len);
  tmp = realloc(buf->data, len);
  if(tmp == NULL)
    return CURLE_OUT_OF_MEMORY;

  buf->data = tmp;
  ret = socket_read(fd, buf->data, len);
  if(ret != CURLE_OK)
    return ret;
  buf->size = conn->mech->decode(conn->app_data, buf->data, len,
                                 conn->data_prot, conn);
  buf->index = 0;
  return CURLE_OK;
}