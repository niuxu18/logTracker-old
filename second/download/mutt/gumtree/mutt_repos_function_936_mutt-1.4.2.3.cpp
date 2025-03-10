void mutt_pretty_mailbox (char *s)
{
  char *p = s, *q = s;
  size_t len;
  url_scheme_t scheme;

  scheme = url_check_scheme (s);

#ifdef USE_IMAP
  if (scheme == U_IMAP || scheme == U_IMAPS)
  {
    imap_pretty_mailbox (s);
    return;
  }
#endif

  /* if s is an url, only collapse path component */
  if (scheme != U_UNKNOWN)
  {
    p = strchr(s, ':')+1;
    if (!strncmp (p, "//", 2))
      q = strchr (p+2, '/');
    if (!q)
      q = strchr (p, '\0');
    p = q;
  }
  
  /* first attempt to collapse the pathname */
  while (*p)
  {
    if (*p == '/' && p[1] == '/')
    {
      *q++ = '/';
      p += 2;
    }
    else if (p[0] == '/' && p[1] == '.' && p[2] == '/')
    {
      *q++ = '/';
      p += 3;
    }
    else
      *q++ = *p++;
  }
  *q = 0;

  if (mutt_strncmp (s, Maildir, (len = mutt_strlen (Maildir))) == 0 &&
      s[len] == '/')
  {
    *s++ = '=';
    memmove (s, s + len, mutt_strlen (s + len) + 1);
  }
  else if (mutt_strncmp (s, Homedir, (len = mutt_strlen (Homedir))) == 0 &&
	   s[len] == '/')
  {
    *s++ = '~';
    memmove (s, s + len - 1, mutt_strlen (s + len - 1) + 1);
  }
}