int mutt_account_getuser (ACCOUNT* account)
{
  char prompt[SHORT_STRING];

  /* already set */
  if (account->flags & M_ACCT_USER)
    return 0;
#ifdef USE_IMAP
  else if ((account->type == M_ACCT_TYPE_IMAP) && ImapUser)
    strfcpy (account->user, ImapUser, sizeof (account->user));
#endif
#ifdef USE_POP
  else if ((account->type == M_ACCT_TYPE_POP) && PopUser)
    strfcpy (account->user, PopUser, sizeof (account->user));
#endif
  /* prompt (defaults to unix username), copy into account->user */
  else
  {
    snprintf (prompt, sizeof (prompt), _("Username at %s: "), account->host);
    strfcpy (account->user, NONULL (Username), sizeof (account->user));
    if (mutt_get_field (prompt, account->user, sizeof (account->user), 0))
      return -1;
  }

  account->flags |= M_ACCT_USER;

  return 0;
}