void mutt_edit_headers (const char *editor,
			const char *body,
			HEADER *msg,
			char *fcc,
			size_t fcclen)
{
  char path[_POSIX_PATH_MAX];	/* tempfile used to edit headers + body */
  char buffer[LONG_STRING];
  char *p;
  FILE *ifp, *ofp;
  int i, keep;
  ENVELOPE *n;
  time_t mtime;
  struct stat st;
  LIST *cur, **last = NULL, *tmp;

  mutt_mktemp (path);
  if ((ofp = safe_fopen (path, "w")) == NULL)
  {
    mutt_perror (path);
    return;
  }

  mutt_write_rfc822_header (ofp, msg->env, NULL, 1, 0);
  fputc ('\n', ofp);	/* tie off the header. */

  /* now copy the body of the message. */
  if ((ifp = fopen (body, "r")) == NULL)
  {
    mutt_perror (body);
    return;
  }

  mutt_copy_stream (ifp, ofp);

  fclose (ifp);
  fclose (ofp);

  if (stat (path, &st) == -1)
  {
    mutt_perror (path);
    return;
  }

  mtime = mutt_decrease_mtime (path, &st);

  mutt_edit_file (editor, path);
  stat (path, &st);
  if (mtime == st.st_mtime)
  {
    dprint (1, (debugfile, "ci_edit_headers(): temp file was not modified.\n"));
    /* the file has not changed! */
    mutt_unlink (path);
    return;
  }

  mutt_unlink (body);
  mutt_free_list (&msg->env->userhdrs);

  /* Read the temp file back in */
  if ((ifp = fopen (path, "r")) == NULL)
  {
    mutt_perror (path);
    return;
  }
  
  if ((ofp = safe_fopen (body, "w")) == NULL)
  {
    /* intentionally leak a possible temporary file here */
    fclose (ifp);
    mutt_perror (body);
    return;
  }
  
  n = mutt_read_rfc822_header (ifp, NULL, 1, 0);
  while ((i = fread (buffer, 1, sizeof (buffer), ifp)) > 0)
    fwrite (buffer, 1, i, ofp);
  fclose (ofp);
  fclose (ifp);
  mutt_unlink (path);

  /* restore old info. */
  n->references = msg->env->references;
  msg->env->references = NULL;

  mutt_free_envelope (&msg->env);
  msg->env = n; n = NULL;

  if (!msg->env->in_reply_to)
    mutt_free_list (&msg->env->references);

  mutt_expand_aliases_env (msg->env);

  /* search through the user defined headers added to see if either a 
   * fcc: or attach-file: field was specified.  
   */

  cur = msg->env->userhdrs;
  last = &msg->env->userhdrs;
  while (cur)
  {
    keep = 1;

    /* keep track of whether or not we see the in-reply-to field.  if we did
     * not, remove the references: field later so that we can generate a new
     * message based upon this one.
     */
    if (fcc && ascii_strncasecmp ("fcc:", cur->data, 4) == 0)
    {
      p = cur->data + 4;
      SKIPWS (p);
      if (*p)
      {
	strfcpy (fcc, p, fcclen);
	mutt_pretty_mailbox (fcc);
      }
      keep = 0;
    }
    else if (ascii_strncasecmp ("attach:", cur->data, 7) == 0)
    {
      BODY *body;
      BODY *parts;
      char *q;

      p = cur->data + 7;
      SKIPWS (p);
      if (*p)
      {
	if ((q = strpbrk (p, " \t")))
	{
	  mutt_substrcpy (path, p, q, sizeof (path));
	  SKIPWS (q);
	}
	else
	  strfcpy (path, p, sizeof (path));
	mutt_expand_path (path, sizeof (path));
	if ((body = mutt_make_file_attach (path)))
	{
	  body->description = safe_strdup (q);
	  for (parts = msg->content; parts->next; parts = parts->next) ;
	  parts->next = body;
	}
	else
	{
	  mutt_pretty_mailbox (path);
	  mutt_error (_("%s: unable to attach file"), path);
	}
      }
      keep = 0;
    }



#ifdef HAVE_PGP
    else if (ascii_strncasecmp ("pgp:", cur->data, 4) == 0)
    {
      msg->pgp = mutt_parse_pgp_hdr (cur->data + 4, 0);
      keep = 0;
    }
#endif


    if (keep)
    {
      last = &cur->next;
      cur  = cur->next;
    }
    else
    {
      tmp       = cur;
      *last     = cur->next;
      cur       = cur->next;
      tmp->next = NULL;
      mutt_free_list (&tmp);
    }
  }
}