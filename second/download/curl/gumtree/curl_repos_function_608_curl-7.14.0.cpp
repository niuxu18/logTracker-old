struct CookieInfo *Curl_cookie_init(struct SessionHandle *data,
                                    char *file,
                                    struct CookieInfo *inc,
                                    bool newsession)
{
  struct CookieInfo *c;
  FILE *fp;
  bool fromfile=TRUE;

  if(NULL == inc) {
    /* we didn't get a struct, create one */
    c = (struct CookieInfo *)calloc(1, sizeof(struct CookieInfo));
    if(!c)
      return NULL; /* failed to get memory */
    c->filename = strdup(file?file:"none"); /* copy the name just in case */
  }
  else {
    /* we got an already existing one, use that */
    c = inc;
  }
  c->running = FALSE; /* this is not running, this is init */

  if(file && strequal(file, "-")) {
    fp = stdin;
    fromfile=FALSE;
  }
  else if(file && !*file) {
    /* points to a "" string */
    fp = NULL;
  }
  else
    fp = file?fopen(file, "r"):NULL;

  c->newsession = newsession; /* new session? */

  if(fp) {
    char *lineptr;
    bool headerline;

    char *line = (char *)malloc(MAX_COOKIE_LINE);
    if(line) {
      while(fgets(line, MAX_COOKIE_LINE, fp)) {
        if(checkprefix("Set-Cookie:", line)) {
          /* This is a cookie line, get it! */
          lineptr=&line[11];
          headerline=TRUE;
        }
        else {
          lineptr=line;
          headerline=FALSE;
        }
        while(*lineptr && my_isspace(*lineptr))
          lineptr++;

        Curl_cookie_add(data, c, headerline, lineptr, NULL, NULL);
      }
      free(line); /* free the line buffer */
    }
    if(fromfile)
      fclose(fp);
  }

  c->running = TRUE;          /* now, we're running */

  return c;
}