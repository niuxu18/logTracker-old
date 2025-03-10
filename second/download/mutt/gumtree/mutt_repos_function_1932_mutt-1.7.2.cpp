int imap_read_literal (FILE* fp, IMAP_DATA* idata, long bytes, progress_t* pbar)
{
  long pos;
  char c;

  int r = 0;

  dprint (2, (debugfile, "imap_read_literal: reading %ld bytes\n", bytes));

  for (pos = 0; pos < bytes; pos++)
  {
    if (mutt_socket_readchar (idata->conn, &c) != 1)
    {
      dprint (1, (debugfile, "imap_read_literal: error during read, %ld bytes read\n", pos));
      idata->status = IMAP_FATAL;

      return -1;
    }

#if 1
    if (r == 1 && c != '\n')
      fputc ('\r', fp);

    if (c == '\r')
    {
      r = 1;
      continue;
    }
    else
      r = 0;
#endif
    fputc (c, fp);

    if (pbar && !(pos % 1024))
      mutt_progress_update (pbar, pos, -1);
#ifdef DEBUG
    if (debuglevel >= IMAP_LOG_LTRL)
      fputc (c, debugfile);
#endif
  }

  return 0;
}