IMAP_DATA* imap_conn_find (const ACCOUNT* account, int flags)
{
  CONNECTION* conn = NULL;
  ACCOUNT* creds = NULL;
  IMAP_DATA* idata = NULL;
  int new = 0;

  while ((conn = mutt_conn_find (conn, account)))
  {
    if (!creds)
      creds = &conn->account;
    else
      memcpy (&conn->account, creds, sizeof (ACCOUNT));

    idata = (IMAP_DATA*)conn->data;
    if (flags & MUTT_IMAP_CONN_NONEW)
    {
      if (!idata)
      {
        /* This should only happen if we've come to the end of the list */
        mutt_socket_free (conn);
        return NULL;
      }
      else if (idata->state < IMAP_AUTHENTICATED)
        continue;
    }
    if (flags & MUTT_IMAP_CONN_NOSELECT && idata && idata->state >= IMAP_SELECTED)
      continue;
    if (idata && idata->status == IMAP_FATAL)
      continue;
    break;
  }
  if (!conn)
    return NULL; /* this happens when the initial connection fails */

  if (!idata)
  {
    /* The current connection is a new connection */
    if (! (idata = imap_new_idata ()))
    {
      mutt_socket_free (conn);
      return NULL;
    }

    conn->data = idata;
    idata->conn = conn;
    new = 1;
  }

  if (idata->state == IMAP_DISCONNECTED)
    imap_open_connection (idata);
  if (idata->state == IMAP_CONNECTED)
  {
    if (!imap_authenticate (idata))
    {
      idata->state = IMAP_AUTHENTICATED;
      FREE (&idata->capstr);
      new = 1;
      if (idata->conn->ssf)
	dprint (2, (debugfile, "Communication encrypted at %d bits\n",
		    idata->conn->ssf));
    }
    else
      mutt_account_unsetpass (&idata->conn->account);
  }
  if (new && idata->state == IMAP_AUTHENTICATED)
  {
    /* capabilities may have changed */
    imap_exec (idata, "CAPABILITY", IMAP_CMD_QUEUE);
    /* enable RFC6855, if the server supports that */
    if (mutt_bit_isset (idata->capabilities, ENABLE))
      imap_exec (idata, "ENABLE UTF8=ACCEPT", IMAP_CMD_QUEUE);
    /* get root delimiter, '/' as default */
    idata->delim = '/';
    imap_exec (idata, "LIST \"\" \"\"", IMAP_CMD_QUEUE);
    if (option (OPTIMAPCHECKSUBSCRIBED))
      imap_exec (idata, "LSUB \"\" \"*\"", IMAP_CMD_QUEUE);
    /* we may need the root delimiter before we open a mailbox */
    imap_exec (idata, NULL, IMAP_CMD_FAIL_OK);
  }

  return idata;
}