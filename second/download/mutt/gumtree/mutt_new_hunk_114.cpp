  size_t msglen;
  gpgme_user_id_t uids = NULL;
  int i, aka = 0;

  state_attach_puts (msg, s);
  state_attach_puts (" ", s);
  /* key is NULL when not present in the user's keyring */
  if (key)
  {
    for (uids = key->uids; uids; uids = uids->next)
    {
      if (uids->revoked)
	continue;
      if (aka)
      {
	msglen = mutt_strlen (msg) - 4;
	for (i = 0; i < msglen; i++)
	  state_attach_puts(" ", s);
	state_attach_puts(_("aka: "), s);
      }
      state_attach_puts (uids->uid, s);
      state_attach_puts ("\n", s);

      aka = 1;
    }
  }
  else
  {
    state_attach_puts (_("KeyID "), s);
    state_attach_puts (sig->fpr, s);
    state_attach_puts ("\n", s);
  }

  msglen = mutt_strlen (msg) - 8;
  for (i = 0; i < msglen; i++)
    state_attach_puts(" ", s);
  state_attach_puts (_("created: "), s);
