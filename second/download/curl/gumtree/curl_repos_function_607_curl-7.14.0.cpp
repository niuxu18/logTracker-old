struct Cookie *
Curl_cookie_add(struct SessionHandle *data,
                /* The 'data' pointer here may be NULL at times, and thus
                   must only be used very carefully for things that can deal
                   with data being NULL. Such as infof() and similar */

                struct CookieInfo *c,
                bool httpheader, /* TRUE if HTTP header-style line */
                char *lineptr,   /* first character of the line */
                char *domain,    /* default domain */
                char *path)      /* full path used when this cookie is set,
                                    used to get default path for the cookie
                                    unless set */
{
  struct Cookie *clist;
  char *what;
  char name[MAX_NAME];
  char *ptr;
  char *semiptr;
  struct Cookie *co;
  struct Cookie *lastc=NULL;
  time_t now = time(NULL);
  bool replace_old = FALSE;
  bool badcookie = FALSE; /* cookies are good by default. mmmmm yummy */

  /* First, alloc and init a new struct for it */
  co = (struct Cookie *)calloc(sizeof(struct Cookie), 1);
  if(!co)
    return NULL; /* bail out if we're this low on memory */

  if(httpheader) {
    /* This line was read off a HTTP-header */
    char *sep;

    what = malloc(MAX_COOKIE_LINE);
    if(!what) {
      free(co);
      return NULL;
    }

    semiptr=strchr(lineptr, ';'); /* first, find a semicolon */

    while(*lineptr && my_isspace(*lineptr))
      lineptr++;

    ptr = lineptr;
    do {
      /* we have a <what>=<this> pair or a 'secure' word here */
      sep = strchr(ptr, '=');
      if(sep && (!semiptr || (semiptr>sep)) ) {
        /*
         * There is a = sign and if there was a semicolon too, which make sure
         * that the semicolon comes _after_ the equal sign.
         */

        name[0]=what[0]=0; /* init the buffers */
        if(1 <= sscanf(ptr, "%" MAX_NAME_TXT "[^;=]=%"
                       MAX_COOKIE_LINE_TXT "[^;\r\n]",
                       name, what)) {
          /* this is a <name>=<what> pair */

          char *whatptr;

          /* Strip off trailing whitespace from the 'what' */
          size_t len=strlen(what);
          while(len && my_isspace(what[len-1])) {
            what[len-1]=0;
            len--;
          }

          /* Skip leading whitespace from the 'what' */
          whatptr=what;
          while(my_isspace(*whatptr)) {
            whatptr++;
          }

          if(strequal("path", name)) {
            co->path=strdup(whatptr);
            if(!co->path) {
              badcookie = TRUE; /* out of memory bad */
              break;
            }
          }
          else if(strequal("domain", name)) {
            /* note that this name may or may not have a preceeding dot, but
               we don't care about that, we treat the names the same anyway */

            const char *domptr=whatptr;
            int dotcount=1;

            /* Count the dots, we need to make sure that there are enough
               of them. */

            if('.' == whatptr[0])
              /* don't count the initial dot, assume it */
              domptr++;

            do {
              domptr = strchr(domptr, '.');
              if(domptr) {
                domptr++;
                dotcount++;
              }
            } while(domptr);

            /* The original Netscape cookie spec defined that this domain name
               MUST have three dots (or two if one of the seven holy TLDs),
               but it seems that these kinds of cookies are in use "out there"
               so we cannot be that strict. I've therefore lowered the check
               to not allow less than two dots. */

            if(dotcount < 2) {
              /* Received and skipped a cookie with a domain using too few
                 dots. */
              badcookie=TRUE; /* mark this as a bad cookie */
              infof(data, "skipped cookie with illegal dotcount domain: %s\n",
                    whatptr);
            }
            else {
              /* Now, we make sure that our host is within the given domain,
                 or the given domain is not valid and thus cannot be set. */

              if('.' == whatptr[0])
                whatptr++; /* ignore preceeding dot */

              if(!domain || tailmatch(whatptr, domain)) {
                const char *tailptr=whatptr;
                if(tailptr[0] == '.')
                  tailptr++;
                co->domain=strdup(tailptr); /* don't prefix w/dots
                                               internally */
                if(!co->domain) {
                  badcookie = TRUE;
                  break;
                }
                co->tailmatch=TRUE; /* we always do that if the domain name was
                                       given */
              }
              else {
                /* we did not get a tailmatch and then the attempted set domain
                   is not a domain to which the current host belongs. Mark as
                   bad. */
                badcookie=TRUE;
                infof(data, "skipped cookie with bad tailmatch domain: %s\n",
                      whatptr);
              }
            }
          }
          else if(strequal("version", name)) {
            co->version=strdup(whatptr);
            if(!co->version) {
              badcookie = TRUE;
              break;
            }
          }
          else if(strequal("max-age", name)) {
            /* Defined in RFC2109:

               Optional.  The Max-Age attribute defines the lifetime of the
               cookie, in seconds.  The delta-seconds value is a decimal non-
               negative integer.  After delta-seconds seconds elapse, the
               client should discard the cookie.  A value of zero means the
               cookie should be discarded immediately.

             */
            co->maxage = strdup(whatptr);
            if(!co->maxage) {
              badcookie = TRUE;
              break;
            }
            co->expires =
              atoi((*co->maxage=='\"')?&co->maxage[1]:&co->maxage[0]) + (long)now;
          }
          else if(strequal("expires", name)) {
            co->expirestr=strdup(whatptr);
            if(!co->expirestr) {
              badcookie = TRUE;
              break;
            }
            co->expires = (long)curl_getdate(what, &now);
          }
          else if(!co->name) {
            co->name = strdup(name);
            co->value = strdup(whatptr);
            if(!co->name || !co->value) {
              badcookie = TRUE;
              break;
            }
          }
          /*
            else this is the second (or more) name we don't know
            about! */
        }
        else {
          /* this is an "illegal" <what>=<this> pair */
        }
      }
      else {
        if(sscanf(ptr, "%" MAX_COOKIE_LINE_TXT "[^;\r\n]",
                  what)) {
          if(strequal("secure", what))
            co->secure = TRUE;
          /* else,
             unsupported keyword without assign! */

        }
      }
      if(!semiptr || !*semiptr) {
        /* we already know there are no more cookies */
        semiptr = NULL;
        continue;
      }

      ptr=semiptr+1;
      while(ptr && *ptr && my_isspace(*ptr))
        ptr++;
      semiptr=strchr(ptr, ';'); /* now, find the next semicolon */

      if(!semiptr && *ptr)
        /* There are no more semicolons, but there's a final name=value pair
           coming up */
        semiptr=strchr(ptr, '\0');
    } while(semiptr);

    if(!badcookie && !co->domain) {
      if(domain) {
        /* no domain was given in the header line, set the default */
        co->domain=strdup(domain);
        if(!co->domain)
          badcookie = TRUE;
      }
    }

    if(!badcookie && !co->path && path) {
      /* no path was given in the header line, set the default  */
      char *endslash = strrchr(path, '/');
      if(endslash) {
        size_t pathlen = endslash-path+1; /* include the ending slash */
        co->path=malloc(pathlen+1); /* one extra for the zero byte */
        if(co->path) {
          memcpy(co->path, path, pathlen);
          co->path[pathlen]=0; /* zero terminate */
        }
        else
          badcookie = TRUE;
      }
    }

    free(what);

    if(badcookie || !co->name) {
      /* we didn't get a cookie name or a bad one,
         this is an illegal line, bail out */
      freecookie(co);
      return NULL;
    }

  }
  else {
    /* This line is NOT a HTTP header style line, we do offer support for
       reading the odd netscape cookies-file format here */
    char *firstptr;
    char *tok_buf;
    int fields;

    if(lineptr[0]=='#') {
      /* don't even try the comments */
      free(co);
      return NULL;
    }
    /* strip off the possible end-of-line characters */
    ptr=strchr(lineptr, '\r');
    if(ptr)
      *ptr=0; /* clear it */
    ptr=strchr(lineptr, '\n');
    if(ptr)
      *ptr=0; /* clear it */

    firstptr=strtok_r(lineptr, "\t", &tok_buf); /* tokenize it on the TAB */

    /* Here's a quick check to eliminate normal HTTP-headers from this */
    if(!firstptr || strchr(firstptr, ':')) {
      free(co);
      return NULL;
    }

    /* Now loop through the fields and init the struct we already have
       allocated */
    for(ptr=firstptr, fields=0; ptr && !badcookie;
        ptr=strtok_r(NULL, "\t", &tok_buf), fields++) {
      switch(fields) {
      case 0:
        if(ptr[0]=='.') /* skip preceeding dots */
          ptr++;
        co->domain = strdup(ptr);
        if(!co->domain)
          badcookie = TRUE;
        break;
      case 1:
        /* This field got its explanation on the 23rd of May 2001 by
           Andr�s Garc�a:

           flag: A TRUE/FALSE value indicating if all machines within a given
           domain can access the variable. This value is set automatically by
           the browser, depending on the value you set for the domain.

           As far as I can see, it is set to true when the cookie says
           .domain.com and to false when the domain is complete www.domain.com
        */
        co->tailmatch=(bool)strequal(ptr, "TRUE"); /* store information */
        break;
      case 2:
        /* It turns out, that sometimes the file format allows the path
           field to remain not filled in, we try to detect this and work
           around it! Andr�s Garc�a made us aware of this... */
        if (strcmp("TRUE", ptr) && strcmp("FALSE", ptr)) {
          /* only if the path doesn't look like a boolean option! */
          co->path = strdup(ptr);
          if(!co->path)
            badcookie = TRUE;
          break;
        }
        /* this doesn't look like a path, make one up! */
        co->path = strdup("/");
        if(!co->path)
          badcookie = TRUE;
        fields++; /* add a field and fall down to secure */
        /* FALLTHROUGH */
      case 3:
        co->secure = (bool)strequal(ptr, "TRUE");
        break;
      case 4:
        co->expires = atoi(ptr);
        break;
      case 5:
        co->name = strdup(ptr);
        if(!co->name)
          badcookie = TRUE;
        break;
      case 6:
        co->value = strdup(ptr);
        if(!co->value)
          badcookie = TRUE;
        break;
      }
    }
    if(6 == fields) {
      /* we got a cookie with blank contents, fix it */
      co->value = strdup("");
      if(!co->value)
        badcookie = TRUE;
      else
        fields++;
    }

    if(!badcookie && (7 != fields))
      /* we did not find the sufficient number of fields */
      badcookie = TRUE;

    if(badcookie) {
      freecookie(co);
      return NULL;
    }

  }

  if(!c->running &&    /* read from a file */
     c->newsession &&  /* clean session cookies */
     !co->expires) {   /* this is a session cookie since it doesn't expire! */
    freecookie(co);
    return NULL;
  }

  co->livecookie = c->running;

  /* now, we have parsed the incoming line, we must now check if this
     superceeds an already existing cookie, which it may if the previous have
     the same domain and path as this */

  clist = c->cookies;
  replace_old = FALSE;
  while(clist) {
    if(strequal(clist->name, co->name)) {
      /* the names are identical */

      if(clist->domain && co->domain) {
        if(strequal(clist->domain, co->domain))
          /* The domains are identical */
          replace_old=TRUE;
      }
      else if(!clist->domain && !co->domain)
        replace_old = TRUE;

      if(replace_old) {
        /* the domains were identical */

        if(clist->path && co->path) {
          if(strequal(clist->path, co->path)) {
            replace_old = TRUE;
          }
          else
            replace_old = FALSE;
        }
        else if(!clist->path && !co->path)
          replace_old = TRUE;
        else
          replace_old = FALSE;

      }

      if(replace_old && !co->livecookie && clist->livecookie) {
        /* Both cookies matched fine, except that the already present
           cookie is "live", which means it was set from a header, while
           the new one isn't "live" and thus only read from a file. We let
           live cookies stay alive */

        /* Free the newcomer and get out of here! */
        freecookie(co);
        return NULL;
      }

      if(replace_old) {
        co->next = clist->next; /* get the next-pointer first */

        /* then free all the old pointers */
        if(clist->name)
          free(clist->name);
        if(clist->value)
          free(clist->value);
        if(clist->domain)
          free(clist->domain);
        if(clist->path)
          free(clist->path);
        if(clist->expirestr)
          free(clist->expirestr);

        if(clist->version)
          free(clist->version);
        if(clist->maxage)
          free(clist->maxage);

        *clist = *co;  /* then store all the new data */

        free(co);   /* free the newly alloced memory */
        co = clist; /* point to the previous struct instead */

        /* We have replaced a cookie, now skip the rest of the list but
           make sure the 'lastc' pointer is properly set */
        do {
          lastc = clist;
          clist = clist->next;
        } while(clist);
        break;
      }
    }
    lastc = clist;
    clist = clist->next;
  }

  if(c->running)
    /* Only show this when NOT reading the cookies from a file */
    infof(data, "%s cookie %s=\"%s\" for domain %s, path %s, expire %d\n",
          replace_old?"Replaced":"Added", co->name, co->value,
          co->domain, co->path, co->expires);

  if(!replace_old) {
    /* then make the last item point on this new one */
    if(lastc)
      lastc->next = co;
    else
      c->cookies = co;
  }

  c->numcookies++; /* one more cookie in the jar */
  return co;
}