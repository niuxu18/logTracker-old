    oldOffset[i-first].length = ctx->hdrs[i]->content->length;

    if (! ctx->hdrs[i]->deleted)
    {
      j++;

      if (ctx->magic == M_MMDF)
      {
	if (fputs (MMDF_SEP, fp) == EOF)
	{
	  mutt_perror (tempfile);
	  mutt_sleep (5);
	  unlink (tempfile);
