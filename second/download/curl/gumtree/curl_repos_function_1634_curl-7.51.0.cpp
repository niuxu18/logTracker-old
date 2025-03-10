static CURLcode pop3_doing(struct connectdata *conn, bool *dophase_done)
{
  CURLcode result = pop3_multi_statemach(conn, dophase_done);

  if(result)
    DEBUGF(infof(conn->data, "DO phase failed\n"));
  else if(*dophase_done) {
    result = pop3_dophase_done(conn, FALSE /* not connected */);

    DEBUGF(infof(conn->data, "DO phase is complete\n"));
  }

  return result;
}