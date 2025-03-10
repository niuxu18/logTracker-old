CURLcode checkpasswd(const char *kind, /* for what purpose */
                     char **userpwd)   /* pointer to allocated string */
{
  char *psep;
  char *osep;

  if(!*userpwd)
    return CURLE_OK;

  /* Attempt to find the password separator */
  psep = strchr(*userpwd, ':');

  /* Attempt to find the options separator */
  osep = strchr(*userpwd, ';');

  if(!psep && **userpwd != ';') {
    /* no password present, prompt for one */
    char passwd[256] = "";
    char prompt[256];
    size_t passwdlen;
    size_t userlen = strlen(*userpwd);
    char *passptr;

    if(osep)
      *osep = '\0';

    /* build a nice-looking prompt */
    curlx_msnprintf(prompt, sizeof(prompt),
                    "Enter %s password for user '%s':",
                    kind, *userpwd);

    /* get password */
    getpass_r(prompt, passwd, sizeof(passwd));
    passwdlen = strlen(passwd);

    if(osep)
      *osep = ';';

    /* extend the allocated memory area to fit the password too */
    passptr = realloc(*userpwd,
                      passwdlen + 1 + /* an extra for the colon */
                      userlen + 1);   /* an extra for the zero */
    if(!passptr)
      return CURLE_OUT_OF_MEMORY;

    /* append the password separated with a colon */
    passptr[userlen] = ':';
    memcpy(&passptr[userlen+1], passwd, passwdlen+1);
    *userpwd = passptr;
  }

  return CURLE_OK;
}