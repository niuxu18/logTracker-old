static int
operate(struct Configurable *config, int argc, char *argv[])
{
  char errorbuffer[CURL_ERROR_SIZE];
  char useragent[128]; /* buah, we don't want a larger default user agent */
  struct ProgressData progressbar;
  struct getout *urlnode;
  struct getout *nextnode;

  struct OutStruct outs;
  struct OutStruct heads;
  struct InStruct input;

  char *url = NULL;

  URLGlob *urls=NULL;
  URLGlob *inglob=NULL;
  int urlnum;
  int infilenum;
  char *outfiles;
  char *infiles; /* might a glob pattern */
  char *uploadfile=NULL; /* a single file, never a glob */

  int separator = 0;

  FILE *infd = stdin;
  bool infdfopen;
  FILE *headerfilep = NULL;
  char *urlbuffer=NULL;
  curl_off_t uploadfilesize; /* -1 means unknown */
  bool stillflags=TRUE;

  bool allocuseragent=FALSE;

  char *httpgetfields=NULL;

  CURL *curl;
  int res = 0;
  int i;
  int up; /* upload file counter within a single upload glob */
  long retry_sleep_default;
  long retry_numretries;
  long retry_sleep;
  long response;
  struct timeval retrystart;

  char *env;
#ifdef CURLDEBUG
  /* this sends all memory debug messages to a logfile named memdump */
  env = curlx_getenv("CURL_MEMDEBUG");
  if(env) {
    /* use the value as file name */
    char *s = strdup(env);
    curl_free(env);
    curl_memdebug(s);
    free(s);
    /* this weird strdup() and stuff here is to make the curl_free() get
       called before the memdebug() as otherwise the memdebug tracing will
       with tracing a free() without an alloc! */
  }
  env = curlx_getenv("CURL_MEMLIMIT");
  if(env) {
    curl_memlimit(atoi(env));
    curl_free(env);
  }
#endif

  memset(&outs,0,sizeof(outs));

  /* we get libcurl info right away */
  curlinfo = curl_version_info(CURLVERSION_NOW);

  errorbuffer[0]=0; /* prevent junk from being output */

  /* setup proper locale from environment */
#ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif

  /* inits */
  if (main_init() != CURLE_OK) {
    helpf("error initializing curl library\n");
    return CURLE_FAILED_INIT;
  }
  config->postfieldsize = -1;
  config->showerror=TRUE;
  config->conf=CONF_DEFAULT;
  config->use_httpget=FALSE;
  config->create_dirs=FALSE;
  config->lastrecvtime = curlx_tvnow();
  config->lastsendtime = curlx_tvnow();

  if(argc>1 &&
     (!curlx_strnequal("--", argv[1], 2) && (argv[1][0] == '-')) &&
     strchr(argv[1], 'q')) {
    /*
     * The first flag, that is not a verbose name, but a shortname
     * and it includes the 'q' flag!
     */
    ;
  }
  else {
    parseconfig(NULL, config);
  }

  if ((argc < 2)  && !config->url_list) {
    helpf(NULL);
    return CURLE_FAILED_INIT;
  }

  /* Parse options */
  for (i = 1; i < argc; i++) {
    if(stillflags &&
       ('-' == argv[i][0])) {
      char *nextarg;
      bool passarg;
      char *origopt=argv[i];

      char *flag = argv[i];

      if(curlx_strequal("--", argv[i]))
        /* this indicates the end of the flags and thus enables the
           following (URL) argument to start with -. */
        stillflags=FALSE;
      else {
        nextarg= (i < argc - 1)? argv[i+1]: NULL;

        res = getparameter(flag, nextarg, &passarg, config);
        if(res) {
          const char *reason = param2text(res);
          if(res != PARAM_HELP_REQUESTED)
            helpf("option %s: %s\n", origopt, reason);
          clean_getout(config);
          return CURLE_FAILED_INIT;
        }

        if(passarg) /* we're supposed to skip this */
          i++;
      }
    }
    else {
      bool used;
      /* just add the URL please */
      res = getparameter((char *)"--url", argv[i], &used, config);
      if(res)
        return res;
    }
  }

  retry_sleep_default = config->retry_delay?
    config->retry_delay*1000:RETRY_SLEEP_DEFAULT; /* ms */
  retry_sleep = retry_sleep_default;

  if((!config->url_list || !config->url_list->url) && !config->list_engines) {
    clean_getout(config);
    helpf("no URL specified!\n");
    return CURLE_FAILED_INIT;
  }
  if(NULL == config->useragent) {
    /* set non-zero default values: */
    snprintf(useragent, sizeof(useragent),
             CURL_NAME "/" CURL_VERSION " (" OS ") " "%s", curl_version());
    config->useragent= useragent;
  }
  else
    allocuseragent = TRUE;

  /* On WIN32 (non-cygwin), we can't set the path to curl-ca-bundle.crt
   * at compile time. So we look here for the file in two ways:
   * 1: look at the environment variable CURL_CA_BUNDLE for a path
   * 2: if #1 isn't found, use the windows API function SearchPath()
   *    to find it along the app's path (includes app's dir and CWD)
   *
   * We support the environment variable thing for non-Windows platforms
   * too. Just for the sake of it.
   */
  if (!config->cacert &&
      !config->capath &&
      !config->insecure_ok) {
    env = curlx_getenv("CURL_CA_BUNDLE");
    if(env)
      GetStr(&config->cacert, env);
    else {
      env = curlx_getenv("SSL_CERT_DIR");
      if(env)
        GetStr(&config->capath, env);
      else {
        env = curlx_getenv("SSL_CERT_FILE");
        if(env)
          GetStr(&config->cacert, env);
      }
    }

    if(env)
      curl_free(env);
#if defined(WIN32) && !defined(__CYGWIN32__)
    else
      FindWin32CACert(config, "curl-ca-bundle.crt");
#endif
  }

  if (config->postfields) {
    if (config->use_httpget) {
      /* Use the postfields data for a http get */
      httpgetfields = strdup(config->postfields);
      free(config->postfields);
      config->postfields = NULL;
      if(SetHTTPrequest(config,
                        (config->conf&CONF_NOBODY?HTTPREQ_HEAD:HTTPREQ_GET),
                        &config->httpreq)) {
        free(httpgetfields);
        return PARAM_BAD_USE;
      }
    }
    else {
      if(SetHTTPrequest(config, HTTPREQ_SIMPLEPOST, &config->httpreq))
        return PARAM_BAD_USE;
    }
  }

  /*
   * Get a curl handle to use for all forthcoming curl transfers.  Cleanup
   * when all transfers are done.
   */
  curl = curl_easy_init();
  if(!curl) {
    clean_getout(config);
    return CURLE_FAILED_INIT;
  }


  if (config->list_engines) {
    struct curl_slist *engines = NULL;

    curl_easy_getinfo(curl, CURLINFO_SSL_ENGINES, &engines);
    list_engines(engines);
    curl_slist_free_all(engines);
    res = CURLE_OK;
    goto quit_curl;
  }

  /* After this point, we should call curl_easy_cleanup() if we decide to bail
   * out from this function! */

  urlnode = config->url_list;

  if(config->headerfile) {
    /* open file for output: */
    if(strcmp(config->headerfile,"-")) {
      heads.filename = config->headerfile;
      headerfilep=NULL;
    }
    else
      headerfilep=stdout;
    heads.stream = headerfilep;
    heads.config = config;
  }

  /* loop through the list of given URLs */
  while(urlnode) {
    char *dourl;

    /* get the full URL (it might be NULL) */
    dourl=urlnode->url;

    url = dourl;

    if(NULL == url) {
      /* This node had no URL, skip it and continue to the next */
      if(urlnode->outfile)
        free(urlnode->outfile);

      /* move on to the next URL */
      nextnode=urlnode->next;
      free(urlnode); /* free the node */
      urlnode = nextnode;
      continue; /* next please */
    }

    /* default output stream is stdout */
    outs.stream = stdout;
    outs.config = config;
    outs.bytes = 0; /* nothing written yet */

    /* save outfile pattern before expansion */
    outfiles = urlnode->outfile?strdup(urlnode->outfile):NULL;

    infiles = urlnode->infile;

    if(!config->globoff && infiles) {
      /* Unless explicitly shut off */
      res = glob_url(&inglob, infiles, &infilenum,
                     config->showerror?
                     (config->errors?config->errors:stderr):NULL);
      if(res != CURLE_OK) {
        clean_getout(config);
        if(outfiles)
          free(outfiles);
        break;
      }
    }

    /* Here's the loop for uploading multiple files within the same
       single globbed string. If no upload, we enter the loop once anyway. */
    for(up = 0;
        (!up && !infiles) ||
          (uploadfile = inglob?
           glob_next_url(inglob):
           (!up?strdup(infiles):NULL));
        up++) {
      uploadfilesize=-1;

      if(!config->globoff) {
        /* Unless explicitly shut off, we expand '{...}' and '[...]'
           expressions and return total number of URLs in pattern set */
        res = glob_url(&urls, dourl, &urlnum,
                       config->showerror?
                       (config->errors?config->errors:stderr):NULL);
        if(res != CURLE_OK) {
          break;
        }
      }
      else
        urlnum = 1; /* without globbing, this is a single URL */

      /* if multiple files extracted to stdout, insert separators! */
      separator= ((!outfiles || curlx_strequal(outfiles, "-")) && urlnum > 1);

      /* Here's looping around each globbed URL */
      for(i = 0;
          (url = urls?glob_next_url(urls):(i?NULL:strdup(url)));
          i++) {
        char *outfile;
        outfile = outfiles?strdup(outfiles):NULL;

        if((urlnode->flags&GETOUT_USEREMOTE) ||
           (outfile && !curlx_strequal("-", outfile)) ) {

          /*
           * We have specified a file name to store the result in, or we have
           * decided we want to use the remote file name.
           */

          if(!outfile) {
            /* Find and get the remote file name */
            char * pc =strstr(url, "://");
            if(pc)
              pc+=3;
            else
              pc=url;
            pc = strrchr(pc, '/');

            if(pc) {
              /* duplicate the string beyond the slash */
              pc++;
              outfile = *pc ? strdup(pc): NULL;
            }
            if(!outfile || !*outfile) {
              helpf("Remote file name has no length!\n");
              res = CURLE_WRITE_ERROR;
              free(url);
              break;
            }
#if defined(__DJGPP__)
            {
              /* This is for DOS, and then we do some major replacing of
                 bad characters in the file name before using it */
              char file1 [PATH_MAX];

              strcpy(file1, msdosify(outfile));
              free (outfile);
              outfile = strdup (rename_if_dos_device_name(file1));
            }
#endif /* __DJGPP__ */
          }
          else if(urls) {
            /* fill '#1' ... '#9' terms from URL pattern */
            char *storefile = outfile;
            outfile = glob_match_url(storefile, urls);
            free(storefile);
            if(!outfile) {
              /* bad globbing */
              warnf(config, "bad output glob!\n");
              free(url);
              res = CURLE_FAILED_INIT;
              break;
            }
          }

          /* Create the directory hierarchy, if not pre-existant to a multiple
             file output call */

          if(config->create_dirs &&
             (-1 == create_dir_hierarchy(outfile)))
            return CURLE_WRITE_ERROR;

          if(config->resume_from_current) {
            /* We're told to continue from where we are now. Get the
               size of the file as it is now and open it for append instead */

            struct_stat fileinfo;

            /* VMS -- Danger, the filesize is only valid for stream files */
            if(0 == stat(outfile, &fileinfo))
              /* set offset to current file size: */
              config->resume_from = fileinfo.st_size;
            else
              /* let offset be 0 */
              config->resume_from = 0;
          }

          outs.filename = outfile;

          if(config->resume_from) {
            outs.init = config->resume_from;
            /* open file for output: */
            outs.stream=(FILE *) fopen(outfile, config->resume_from?"ab":"wb");
            if (!outs.stream) {
              helpf("Can't open '%s'!\n", outfile);
              return CURLE_WRITE_ERROR;
            }
          }
          else {
            outs.stream = NULL; /* open when needed */
          }
        }
        infdfopen=FALSE;
        if(uploadfile && !curlx_strequal(uploadfile, "-")) {
          /*
           * We have specified a file to upload and it isn't "-".
           */
          struct_stat fileinfo;

          /* If no file name part is given in the URL, we add this file name */
          char *ptr=strstr(url, "://");
          if(ptr)
            ptr+=3;
          else
            ptr=url;
          ptr = strrchr(ptr, '/');
          if(!ptr || !strlen(++ptr)) {
            /* The URL has no file name part, add the local file name. In order
               to be able to do so, we have to create a new URL in another
               buffer.*/

            /* We only want the part of the local path that is on the right
               side of the rightmost slash and backslash. */
            char *filep = strrchr(uploadfile, '/');
            char *file2 = strrchr(filep?filep:uploadfile, '\\');

            if(file2)
              filep = file2+1;
            else if(filep)
              filep++;
            else
              filep = uploadfile;

            /* URL encode the file name */
            filep = curl_escape(filep, 0 /* use strlen */);

            if(filep) {

              urlbuffer=(char *)malloc(strlen(url) + strlen(filep) + 3);
              if(!urlbuffer) {
                helpf("out of memory\n");
                return CURLE_OUT_OF_MEMORY;
              }
              if(ptr)
                /* there is a trailing slash on the URL */
                sprintf(urlbuffer, "%s%s", url, filep);
              else
                /* thers is no trailing slash on the URL */
                sprintf(urlbuffer, "%s/%s", url, filep);

              curl_free(filep);

              free(url);
              url = urlbuffer; /* use our new URL instead! */
            }
          }
          /* VMS Note:
           *
           * Reading binary from files can be a problem...  Only FIXED, VAR
           * etc WITHOUT implied CC will work Others need a \n appended to a
           * line
           *
           * - Stat gives a size but this is UNRELIABLE in VMS As a f.e. a
           * fixed file with implied CC needs to have a byte added for every
           * record processed, this can by derived from Filesize & recordsize
           * for VARiable record files the records need to be counted!  for
           * every record add 1 for linefeed and subtract 2 for the record
           * header for VARIABLE header files only the bare record data needs
           * to be considered with one appended if implied CC
           */

          infd=(FILE *) fopen(uploadfile, "rb");
          if (!infd || stat(uploadfile, &fileinfo)) {
            helpf("Can't open '%s'!\n", uploadfile);
            return CURLE_READ_ERROR;
          }
          infdfopen=TRUE;
          uploadfilesize=fileinfo.st_size;

        }
        else if(uploadfile && curlx_strequal(uploadfile, "-")) {
          infd = stdin;
        }

        if(uploadfile && config->resume_from_current)
          config->resume_from = -1; /* -1 will then force get-it-yourself */

        if(output_expected(url, uploadfile)
           && outs.stream && isatty(fileno(outs.stream)))
          /* we send the output to a tty, therefore we switch off the progress
             meter */
          config->conf |= CONF_NOPROGRESS;

        if (urlnum > 1 && !(config->conf&CONF_MUTE)) {
          fprintf(stderr, "\n[%d/%d]: %s --> %s\n",
                  i+1, urlnum, url, outfile ? outfile : "<stdout>");
          if (separator)
            printf("%s%s\n", CURLseparator, url);
        }
        if (httpgetfields) {
          /* Find out whether the url contains a file name */
          const char *pc =strstr(url, "://");
          char sep='?';
          if(pc)
            pc+=3;
          else
            pc=url;

          pc = strrchr(pc, '/'); /* check for a slash */

          if(pc) {
            /* there is a slash present in the URL */

            if(strchr(pc, '?'))
              /* Ouch, there's already a question mark in the URL string, we
                 then append the data with an ampersand separator instead! */
              sep='&';
          }
          /*
           * Then append ? followed by the get fields to the url.
           */
          urlbuffer=(char *)malloc(strlen(url) + strlen(httpgetfields) + 2);
          if(!urlbuffer) {
            helpf("out of memory\n");
            return CURLE_OUT_OF_MEMORY;
          }
          if (pc)
            sprintf(urlbuffer, "%s%c%s", url, sep, httpgetfields);
          else
            /* Append  / before the ? to create a well-formed url
               if the url contains a hostname only
            */
            sprintf(urlbuffer, "%s/?%s", url, httpgetfields);

          free(url); /* free previous URL */
          url = urlbuffer; /* use our new URL instead! */
        }

        if(!config->errors)
          config->errors = stderr;

#ifdef O_BINARY
        if(!outfile && !(config->conf & CONF_GETTEXT)) {
          /* We get the output to stdout and we have not got the ASCII/text flag,
             then set stdout to be binary */
          setmode( fileno(stdout), O_BINARY );
        }
#endif

        if(1 == config->tcp_nodelay)
          curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);

        /* where to store */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (FILE *)&outs);
        /* what call to write */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);

        /* for uploads */
        input.stream = infd;
        input.config = config;
        curl_easy_setopt(curl, CURLOPT_READDATA, &input);
        /* what call to read */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, my_fread);

        /* libcurl 7.12.3 business: */
        curl_easy_setopt(curl, CURLOPT_IOCTLDATA, &input);
        curl_easy_setopt(curl, CURLOPT_IOCTLFUNCTION, my_ioctl);

        if(config->recvpersecond) {
          /* tell libcurl to use a smaller sized buffer as it allows us to
             make better sleeps! 7.9.9 stuff! */
          curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, config->recvpersecond);
        }

        /* size of uploaded file: */
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, uploadfilesize);
        curl_easy_setopt(curl, CURLOPT_URL, url);     /* what to fetch */
        curl_easy_setopt(curl, CURLOPT_PROXY, config->proxy); /* proxy to use */
        curl_easy_setopt(curl, CURLOPT_HEADER, config->conf&CONF_HEADER);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, config->conf&CONF_NOPROGRESS);
        curl_easy_setopt(curl, CURLOPT_NOBODY, config->conf&CONF_NOBODY);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR,
                         config->conf&CONF_FAILONERROR);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, uploadfile?TRUE:FALSE);
        curl_easy_setopt(curl, CURLOPT_FTPLISTONLY,
                         config->conf&CONF_FTPLISTONLY);
        curl_easy_setopt(curl, CURLOPT_FTPAPPEND, config->conf&CONF_FTPAPPEND);

        if (config->conf&CONF_NETRC_OPT)
          curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_OPTIONAL);
        else if (config->conf&CONF_NETRC)
          curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_REQUIRED);
        else
          curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,
                         config->conf&CONF_FOLLOWLOCATION);
        curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH,
                         config->conf&CONF_UNRESTRICTED_AUTH);
        curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, config->conf&CONF_GETTEXT);
        curl_easy_setopt(curl, CURLOPT_USERPWD, config->userpwd);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, config->proxyuserpwd);
        curl_easy_setopt(curl, CURLOPT_RANGE, config->range);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config->timeout);

        switch(config->httpreq) {
        case HTTPREQ_SIMPLEPOST:
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, config->postfields);
          curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, config->postfieldsize);
          break;
        case HTTPREQ_POST:
          curl_easy_setopt(curl, CURLOPT_HTTPPOST, config->httppost);
          break;
        default:
          break;
        }
        curl_easy_setopt(curl, CURLOPT_REFERER, config->referer);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER,
                         config->conf&CONF_AUTO_REFERER);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, config->useragent);
        curl_easy_setopt(curl, CURLOPT_FTPPORT, config->ftpport);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, config->low_speed_limit);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, config->low_speed_time);
        curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE,
                         config->use_resume?config->resume_from:0);
        curl_easy_setopt(curl, CURLOPT_COOKIE, config->cookie);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, config->headers);
        curl_easy_setopt(curl, CURLOPT_SSLCERT, config->cert);
        curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, config->cert_type);
        curl_easy_setopt(curl, CURLOPT_SSLKEY, config->key);
        curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, config->key_type);
        curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, config->key_passwd);

        /* default to strict verifyhost */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
        if(config->cacert || config->capath) {
          if (config->cacert)
            curl_easy_setopt(curl, CURLOPT_CAINFO, config->cacert);

          if (config->capath)
            curl_easy_setopt(curl, CURLOPT_CAPATH, config->capath);
          curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, TRUE);
        }
        if(config->insecure_ok) {
          /* new stuff needed for libcurl 7.10 */
          curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
          curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        }

        if((config->conf&CONF_NOBODY) ||
           config->remote_time) {
          /* no body or use remote time */
          curl_easy_setopt(curl, CURLOPT_FILETIME, TRUE);
        }

        if (config->maxredirs)
          curl_easy_setopt(curl, CURLOPT_MAXREDIRS, config->maxredirs);
        else
          curl_easy_setopt(curl, CURLOPT_MAXREDIRS, DEFAULT_MAXREDIRS);

        curl_easy_setopt(curl, CURLOPT_CRLF, config->crlf);
        curl_easy_setopt(curl, CURLOPT_QUOTE, config->quote);
        curl_easy_setopt(curl, CURLOPT_POSTQUOTE, config->postquote);
        curl_easy_setopt(curl, CURLOPT_PREQUOTE, config->prequote);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER,
                         config->headerfile?&heads:NULL);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, config->cookiefile);
        /* cookie jar was added in 7.9 */
        if(config->cookiejar)
          curl_easy_setopt(curl, CURLOPT_COOKIEJAR, config->cookiejar);
        /* cookie session added in 7.9.7 */
        curl_easy_setopt(curl, CURLOPT_COOKIESESSION, config->cookiesession);

        curl_easy_setopt(curl, CURLOPT_SSLVERSION, config->ssl_version);
        curl_easy_setopt(curl, CURLOPT_TIMECONDITION, config->timecond);
        curl_easy_setopt(curl, CURLOPT_TIMEVALUE, config->condtime);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, config->customrequest);
        curl_easy_setopt(curl, CURLOPT_STDERR, config->errors);

        /* three new ones in libcurl 7.3: */
        curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, config->proxytunnel);
        curl_easy_setopt(curl, CURLOPT_INTERFACE, config->iface);
        curl_easy_setopt(curl, CURLOPT_KRB4LEVEL, config->krb4level);

        progressbarinit(&progressbar, config);
        if((config->progressmode == CURL_PROGRESS_BAR) &&
           !(config->conf&(CONF_NOPROGRESS|CONF_MUTE))) {
          /* we want the alternative style, then we have to implement it
             ourselves! */
          curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, myprogress);
          curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progressbar);
        }

        /* new in libcurl 7.6.2: */
        curl_easy_setopt(curl, CURLOPT_TELNETOPTIONS, config->telnet_options);

        /* new in libcurl 7.7: */
        curl_easy_setopt(curl, CURLOPT_RANDOM_FILE, config->random_file);
        curl_easy_setopt(curl, CURLOPT_EGDSOCKET, config->egd_file);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, config->connecttimeout);

        if(config->cipher_list)
          curl_easy_setopt(curl, CURLOPT_SSL_CIPHER_LIST, config->cipher_list);

        if(config->httpversion)
          curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, config->httpversion);

        /* new in libcurl 7.9.2: */
        if(config->disable_epsv)
          /* disable it */
          curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, FALSE);

        /* new in libcurl 7.10.5 */
        if(config->disable_eprt)
          /* disable it */
          curl_easy_setopt(curl, CURLOPT_FTP_USE_EPRT, FALSE);

        /* new in libcurl 7.10.6 (default is Basic) */
        if(config->authtype)
          curl_easy_setopt(curl, CURLOPT_HTTPAUTH, config->authtype);

        /* new in curl 7.9.7 */
        if(config->trace_dump) {
          curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
          curl_easy_setopt(curl, CURLOPT_DEBUGDATA, config);
          curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
        }

        res = CURLE_OK;

        /* new in curl ?? */
        if (config->engine) {
          res = curl_easy_setopt(curl, CURLOPT_SSLENGINE, config->engine);
          curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1);
        }

        if (res != CURLE_OK)
           goto show_error;

        /* new in curl 7.10 */
        curl_easy_setopt(curl, CURLOPT_ENCODING,
                         (config->encoding) ? "" : NULL);

        /* new in curl 7.10.7 */
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS,
                         config->ftp_create_dirs);
        if(config->proxyanyauth)
          curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        else if(config->proxyntlm)
          curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);
        else if(config->proxydigest)
          curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_DIGEST);
        else if(config->proxybasic)
          curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);

        /* new in curl 7.10.8 */
        if(config->max_filesize)
          curl_easy_setopt(curl, CURLOPT_MAXFILESIZE_LARGE,
                           config->max_filesize);

        if(4 == config->ip_version)
          curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        else if(6 == config->ip_version)
          curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
        else
          curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);

        /* new in curl 7.11.0 */
        if(config->ftp_ssl)
          curl_easy_setopt(curl, CURLOPT_FTP_SSL, CURLFTPSSL_TRY);

        /* new in curl 7.11.1 */
        if(config->socks5proxy) {
          curl_easy_setopt(curl, CURLOPT_PROXY, config->socks5proxy);
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
        }

        /* curl 7.13.0 */
        curl_easy_setopt(curl, CURLOPT_SOURCE_URL, config->tp_url);
        curl_easy_setopt(curl, CURLOPT_SOURCE_USERPWD, config->tp_user);
        curl_easy_setopt(curl, CURLOPT_SOURCE_PREQUOTE, config->tp_prequote);
        curl_easy_setopt(curl, CURLOPT_SOURCE_POSTQUOTE, config->tp_postquote);
        curl_easy_setopt(curl, CURLOPT_SOURCE_QUOTE, config->tp_quote);
        curl_easy_setopt(curl, CURLOPT_FTP_ACCOUNT, config->ftp_account);

        curl_easy_setopt(curl, CURLOPT_IGNORE_CONTENT_LENGTH, config->ignorecl);

        /* curl 7.14.2 */
        curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP,
                         config->ftp_skip_ip);

        retry_numretries = config->req_retry;

        retrystart = curlx_tvnow();

        do {
          res = curl_easy_perform(curl);

          /* if retry-max-time is non-zero, make sure we haven't exceeded the
             time */
          if(retry_numretries &&
             (!config->retry_maxtime ||
              (curlx_tvdiff(curlx_tvnow(), retrystart)<
               config->retry_maxtime*1000)) ) {
            enum {
              RETRY_NO,
              RETRY_TIMEOUT,
              RETRY_HTTP,
              RETRY_FTP,
              RETRY_LAST /* not used */
            } retry = RETRY_NO;
            if(CURLE_OPERATION_TIMEDOUT == res)
              /* retry timeout always */
              retry = RETRY_TIMEOUT;
            else if(CURLE_OK == res) {
              /* Check for HTTP transient errors */
              char *url=NULL;
              curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
              if(url &&
                 curlx_strnequal(url, "http", 4)) {
                /* This was HTTP(S) */
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);

                switch(response) {
                case 500: /* Internal Server Error */
                case 502: /* Bad Gateway */
                case 503: /* Service Unavailable */
                case 504: /* Gateway Timeout */
                  retry = RETRY_HTTP;
                  /*
                   * At this point, we have already written data to the output
                   * file (or terminal). If we write to a file, we must rewind
                   * or close/re-open the file so that the next attempt starts
                   * over from the beginning.
                   *
                   * TODO: similar action for the upload case. We might need
                   * to start over reading from a previous point if we have
                   * uploaded something when this was returned.
                   */
                  break;
                }
              }
            } /* if CURLE_OK */
            else if((CURLE_FTP_USER_PASSWORD_INCORRECT == res) ||
                    (CURLE_LOGIN_DENIED == res)) {
              curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);

              if(response/100 == 5)
                /*
                 * This is typically when the FTP server only allows a certain
                 * amount of users and we are not one of them. It mostly
                 * returns 530 in this case, but all 5xx codes are transient.
                 */
                retry = RETRY_FTP;
            }

            if(retry) {
              static const char * const m[]={NULL,
                                             "timeout",
                                             "HTTP error",
                                             "FTP error"
              };
              warnf(config, "Transient problem: %s "
                    "Will retry in %ld seconds. "
                    "%ld retries left.\n",
                    m[retry],
                    retry_sleep/1000,
                    retry_numretries);

              go_sleep(retry_sleep);
              retry_numretries--;
              if(!config->retry_delay) {
                retry_sleep *= 2;
                if(retry_sleep > RETRY_SLEEP_MAX)
                  retry_sleep = RETRY_SLEEP_MAX;
              }
              if(outs.bytes && outs.filename) {
                /* We have written data to a output file, we truncate file
                 */
                if(!(config->conf&CONF_MUTE))
                  fprintf(stderr, "Throwing away " CURL_FORMAT_OFF_T
                          " bytes\n", outs.bytes);
                fflush(outs.stream);
                /* truncate file at the position where we started appending */
#ifdef HAVE_FTRUNCATE
                ftruncate( fileno(outs.stream), outs.init);
                /* now seek to the end of the file, the position where we
                   just truncated the file in a large file-safe way */
                fseek(outs.stream, 0, SEEK_END);
#else
                /* ftruncate is not available, so just reposition the file
                   to the location we would have truncated it. This won't
                   work properly with large files on 32-bit systems, but
                   most of those will have ftruncate. */
                fseek(outs.stream, (long)outs.init, SEEK_SET);
#endif
                outs.bytes = 0; /* clear for next round */
              }
              continue;
            }
          } /* if retry_numretries */

          /* In all ordinary cases, just break out of loop here */
          retry_sleep = retry_sleep_default;
          break;

        } while(1);

        if((config->progressmode == CURL_PROGRESS_BAR) &&
           progressbar.calls) {
          /* if the custom progress bar has been displayed, we output a
             newline here */
          fputs("\n", progressbar.out);
        }

        if(config->writeout) {
          ourWriteOut(curl, config->writeout);
        }
#ifdef USE_ENVIRONMENT
        if (config->writeenv)
          ourWriteEnv(curl);
#endif

show_error:

#ifdef  VMS
        if (!config->showerror)  {
          vms_show = VMSSTS_HIDE;
        }
#else
        if((res!=CURLE_OK) && config->showerror) {
          fprintf(config->errors, "curl: (%d) %s\n", (int)res,
                  errorbuffer[0]? errorbuffer:
                  curl_easy_strerror((CURLcode)res));
          if(CURLE_SSL_CACERT == res) {
#define CURL_CA_CERT_ERRORMSG1 \
"More details here: http://curl.haxx.se/docs/sslcerts.html\n\n" \
"curl performs SSL certificate verification by default, using a \"bundle\"\n" \
" of Certificate Authority (CA) public keys (CA certs). The default\n" \
" bundle is named curl-ca-bundle.crt; you can specify an alternate file\n" \
" using the --cacert option.\n"

#define CURL_CA_CERT_ERRORMSG2 \
"If this HTTPS server uses a certificate signed by a CA represented in\n" \
" the bundle, the certificate verification probably failed due to a\n" \
" problem with the certificate (it might be expired, or the name might\n" \
" not match the domain name in the URL).\n" \
"If you'd like to turn off curl's verification of the certificate, use\n" \
" the -k (or --insecure) option.\n"

            fprintf(config->errors, "%s%s",
                    CURL_CA_CERT_ERRORMSG1,
                    CURL_CA_CERT_ERRORMSG2 );
          }
        }
#endif

        if (outfile && !curlx_strequal(outfile, "-") && outs.stream)
          fclose(outs.stream);

#ifdef HAVE_UTIME
        /* Important that we set the time _after_ the file has been
           closed, as is done above here */
        if(config->remote_time && outs.filename) {
          /* as libcurl if we got a time. Pretty please */
          long filetime;
          curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
          if(filetime >= 0) {
            struct utimbuf times;
            times.actime = (time_t)filetime;
            times.modtime = (time_t)filetime;
            utime(outs.filename, &times); /* set the time we got */
          }
        }
#endif
#ifdef AMIGA
        /* Set the url as comment for the file. (up to 80 chars are allowed)
         */
        if( strlen(url) > 78 )
          url[79] = '\0';

        SetComment( outs.filename, url);
#endif

        if(headerfilep)
          fclose(headerfilep);

        if(url)
          free(url);

        if(outfile)
          free(outfile);

        if(infdfopen)
          fclose(infd);

      } /* loop to the next URL */

      if(urls)
        /* cleanup memory used for URL globbing patterns */
        glob_cleanup(urls);

      if(uploadfile)
        free(uploadfile);

    } /* loop to the next globbed upload file */

    if(inglob)
      glob_cleanup(inglob);

    if(outfiles)
      free(outfiles);

    /* empty this urlnode struct */
    if(urlnode->url)
      free(urlnode->url);
    if(urlnode->outfile)
      free(urlnode->outfile);
    if(urlnode->infile)
      free(urlnode->infile);

    /* move on to the next URL */
    nextnode=urlnode->next;
    free(urlnode); /* free the node */
    urlnode = nextnode;

  } /* while-loop through all URLs */

quit_curl:
  if (httpgetfields)
    free(httpgetfields);

  if (config->engine)
    free(config->engine);

  /* cleanup the curl handle! */
  curl_easy_cleanup(curl);

  if(config->headerfile && !headerfilep && heads.stream)
    fclose(heads.stream);

  if(allocuseragent)
    free(config->useragent);

  if(config->trace_fopened && config->trace_stream)
    fclose(config->trace_stream);

  if(config->errors_fopened)
    fclose(config->errors);

  main_free(); /* cleanup */

  return res;
}