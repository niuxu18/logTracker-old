static void negotiate(struct connectdata *conn)
{
  int i;
  struct TELNET *tn = (struct TELNET *) conn->data->state.proto.telnet;

  for(i = 0;i < CURL_NTELOPTS;i++)
  {
    if(tn->us_preferred[i] == CURL_YES)
      set_local_option(conn, i, CURL_YES);

    if(tn->him_preferred[i] == CURL_YES)
      set_remote_option(conn, i, CURL_YES);
  }
}