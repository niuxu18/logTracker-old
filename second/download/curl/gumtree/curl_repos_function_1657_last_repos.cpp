int Curl_parsenetrc(const char *host,
                    char **loginp,
                    char **passwordp,
                    char *netrcfile)
{
  FILE *file;
  int retcode = 1;
  int specific_login = (*loginp && **loginp != 0);
  bool netrc_alloc = FALSE;
  enum host_lookup_state state = NOTHING;

  char state_login = 0;      /* Found a login keyword */
  char state_password = 0;   /* Found a password keyword */
  int state_our_login = FALSE;  /* With specific_login, found *our* login
                                   name */

#define NETRC DOT_CHAR "netrc"

  if(!netrcfile) {
    bool home_alloc = FALSE;
    char *home = curl_getenv("HOME"); /* portable environment reader */
    if(home) {
      home_alloc = TRUE;
#if defined(HAVE_GETPWUID_R) && defined(HAVE_GETEUID)
    }
    else {
      struct passwd pw, *pw_res;
      char pwbuf[1024];
      if(!getpwuid_r(geteuid(), &pw, pwbuf, sizeof(pwbuf), &pw_res)
         && pw_res) {
        home = strdup(pw.pw_dir);
        if(!home)
          return CURLE_OUT_OF_MEMORY;
        home_alloc = TRUE;
      }
#elif defined(HAVE_GETPWUID) && defined(HAVE_GETEUID)
    }
    else {
      struct passwd *pw;
      pw = getpwuid(geteuid());
      if(pw) {
        home = pw->pw_dir;
      }
#endif
    }

    if(!home)
      return retcode; /* no home directory found (or possibly out of memory) */

    netrcfile = curl_maprintf("%s%s%s", home, DIR_CHAR, NETRC);
    if(home_alloc)
      free(home);
    if(!netrcfile) {
      return -1;
    }
    netrc_alloc = TRUE;
  }

  file = fopen(netrcfile, FOPEN_READTEXT);
  if(netrc_alloc)
    free(netrcfile);
  if(file) {
    char *tok;
    char *tok_buf;
    bool done = FALSE;
    char netrcbuffer[256];
    int  netrcbuffsize = (int)sizeof(netrcbuffer);

    while(!done && fgets(netrcbuffer, netrcbuffsize, file)) {
      tok = strtok_r(netrcbuffer, " \t\n", &tok_buf);
      if(tok && *tok == '#')
        /* treat an initial hash as a comment line */
        continue;
      while(!done && tok) {

        if((*loginp && **loginp) && (*passwordp && **passwordp)) {
          done = TRUE;
          break;
        }

        switch(state) {
        case NOTHING:
          if(strcasecompare("machine", tok)) {
            /* the next tok is the machine name, this is in itself the
               delimiter that starts the stuff entered for this machine,
               after this we need to search for 'login' and
               'password'. */
            state = HOSTFOUND;
          }
          else if(strcasecompare("default", tok)) {
            state = HOSTVALID;
            retcode = 0; /* we did find our host */
          }
          break;
        case HOSTFOUND:
          if(strcasecompare(host, tok)) {
            /* and yes, this is our host! */
            state = HOSTVALID;
            retcode = 0; /* we did find our host */
          }
          else
            /* not our host */
            state = NOTHING;
          break;
        case HOSTVALID:
          /* we are now parsing sub-keywords concerning "our" host */
          if(state_login) {
            if(specific_login) {
              state_our_login = strcasecompare(*loginp, tok);
            }
            else {
              free(*loginp);
              *loginp = strdup(tok);
              if(!*loginp) {
                retcode = -1; /* allocation failed */
                goto out;
              }
            }
            state_login = 0;
          }
          else if(state_password) {
            if(state_our_login || !specific_login) {
              free(*passwordp);
              *passwordp = strdup(tok);
              if(!*passwordp) {
                retcode = -1; /* allocation failed */
                goto out;
              }
            }
            state_password = 0;
          }
          else if(strcasecompare("login", tok))
            state_login = 1;
          else if(strcasecompare("password", tok))
            state_password = 1;
          else if(strcasecompare("machine", tok)) {
            /* ok, there's machine here go => */
            state = HOSTFOUND;
            state_our_login = FALSE;
          }
          break;
        } /* switch (state) */

        tok = strtok_r(NULL, " \t\n", &tok_buf);
      } /* while(tok) */
    } /* while fgets() */

    out:
    fclose(file);
  }

  return retcode;
}