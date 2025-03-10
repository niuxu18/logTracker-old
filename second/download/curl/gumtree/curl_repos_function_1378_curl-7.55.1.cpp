char *Curl_checkProxyheaders(const struct connectdata *conn,
                             const char *thisheader)
{
  struct curl_slist *head;
  size_t thislen = strlen(thisheader);
  struct Curl_easy *data = conn->data;

  for(head = (conn->bits.proxy && data->set.sep_headers) ?
        data->set.proxyheaders : data->set.headers;
      head; head=head->next) {
    if(strncasecompare(head->data, thisheader, thislen))
      return head->data;
  }

  return NULL;
}