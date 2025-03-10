void imap_logout (IMAP_DATA* idata)
{
  /* we set status here to let imap_handle_untagged know we _expect_ to
   * receive a bye response (so it doesn't freak out and close the conn) */
  idata->status = IMAP_BYE;
  imap_cmd_start (idata, "LOGOUT");
  while (imap_cmd_step (idata) == IMAP_CMD_CONTINUE)
    ;
}