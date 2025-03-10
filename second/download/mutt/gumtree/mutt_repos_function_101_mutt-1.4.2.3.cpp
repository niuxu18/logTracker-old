int mutt_pattern_func (int op, char *prompt)
{
  pattern_t *pat;
  char buf[LONG_STRING] = "", *simple, error[STRING];
  BUFFER err;
  int i;

  strfcpy (buf, NONULL (Context->pattern), sizeof (buf));
  if (mutt_get_field (prompt, buf, sizeof (buf), M_PATTERN | M_CLEAR) != 0 || !buf[0])
    return (-1);

  mutt_message _("Compiling search pattern...");
  
  simple = safe_strdup (buf);
  mutt_check_simple (buf, sizeof (buf), NONULL (SimpleSearch));

  err.data = error;
  err.dsize = sizeof (error);
  if ((pat = mutt_pattern_comp (buf, M_FULL_MSG, &err)) == NULL)
  {
    FREE (&simple);
    mutt_error ("%s", err.data);
    return (-1);
  }

  mutt_message _("Executing command on matching messages...");

#define THIS_BODY Context->hdrs[i]->content

  if (op == M_LIMIT)
  {
    Context->vcount    = 0;
    Context->vsize     = 0;
    Context->collapsed = 0;

    for (i = 0; i < Context->msgcount; i++)
    {
      /* new limit pattern implicitly uncollapses all threads */
      Context->hdrs[i]->virtual = -1;
      Context->hdrs[i]->limited = 0;
      Context->hdrs[i]->collapsed = 0;
      Context->hdrs[i]->num_hidden = 0;
      if (mutt_pattern_exec (pat, M_MATCH_FULL_ADDRESS, Context, Context->hdrs[i]))
      {
	Context->hdrs[i]->virtual = Context->vcount;
	Context->hdrs[i]->limited = 1;
	Context->v2r[Context->vcount] = i;
	Context->vcount++;
	Context->vsize+=THIS_BODY->length + THIS_BODY->offset -
	  THIS_BODY->hdr_offset;
      }
    }
  }
  else
  {
    for (i = 0; i < Context->vcount; i++)
    {
      if (mutt_pattern_exec (pat, M_MATCH_FULL_ADDRESS, Context, Context->hdrs[Context->v2r[i]]))
      {
	switch (op)
	{
	  case M_DELETE:
	  case M_UNDELETE:
	    mutt_set_flag (Context, Context->hdrs[Context->v2r[i]], M_DELETE, 
			  (op == M_DELETE));
	    break;
	  case M_TAG:
	  case M_UNTAG:
	    mutt_set_flag (Context, Context->hdrs[Context->v2r[i]], M_TAG, 
			   (op == M_TAG));
	    break;
	}
      }
    }
  }

#undef THIS_BODY

  mutt_clear_error ();

  if (op == M_LIMIT)
  {
    safe_free ((void **) &Context->pattern);
    if (Context->limit_pattern) 
      mutt_pattern_free (&Context->limit_pattern);
    if (!Context->vcount)
    {
      Context->vcount = Context->msgcount;
      mutt_error _("No messages matched criteria.");
      /* restore full display */
      for (i = 0; i < Context->msgcount; i++)
      {
	Context->hdrs[i]->virtual = i;
	Context->v2r[i] = i;
      }
    }
    else if (mutt_strncmp (buf, "~A", 2) != 0)
    {
      Context->pattern = simple;
      simple = NULL; /* don't clobber it */
      Context->limit_pattern = mutt_pattern_comp (buf, M_FULL_MSG, &err);
    }
  }
  FREE (&simple);
  mutt_pattern_free (&pat);
  return 0;
}