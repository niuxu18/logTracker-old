static int
operate(struct Configurable *config, int argc, argv_item_t argv[])
{
  char errorbuffer[CURL_ERROR_SIZE];
  char useragent[256]; /* buah, we don't want a larger default user agent */
  struct ProgressData progressbar;
  struct getout *urlnode;
  struct getout *nextnode;

  struct OutStruct outs;
  struct OutStruct heads;
  struct InStruct input;

  URLGlob *urls=NULL;
  URLGlob *inglob=NULL;
  int urlnum;
  int infilenum;
  char *uploadfile=NULL; /* a single file, never a glob */

  curl_off_t uploadfilesize; /* -1 means unknown */
  bool stillflags=TRUE;

  bool allocuseragent=FALSE;

  char *httpgetfields=NULL;

  CURL *curl;
  int res = 0;
  int i;
  long retry_sleep_default;
  long retry_sleep;

  char *env;

  memset(&heads, 0, sizeof(struct OutStruct));

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
    char *endptr;
    long num = strtol(env, &endptr, 10);
    if((endptr != env) && (endptr == env + strlen(env)) && (num > 0))
      curl_memlimit(num);
    curl_free(env);
  }
#endif

  /* Initialize curl library - do not call any libcurl functions before.
     Note that the CURLDEBUG magic above is an exception, but then that's not
     part of the official public API.
  */
  if(main_init() != CURLE_OK) {
    helpf(config->errors, "error initializing curl library\n");
    return CURLE_FAILED_INIT;
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
  config->easy = curl;

  memset(&outs,0,sizeof(outs));

  config->outs = &outs;

  /* we get libcurl info right away */
  curlinfo = curl_version_info(CURLVERSION_NOW);

  errorbuffer[0]=0; /* prevent junk from being output */

  /* setup proper locale from environment */
#ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif

  /* inits */
  config->postfieldsize = -1;
  config->showerror=TRUE;
  config->use_httpget=FALSE;
  config->create_dirs=FALSE;
  config->maxredirs = DEFAULT_MAXREDIRS;
  config->proto = CURLPROTO_ALL; /* FIXME: better to read from library */
  config->proto_present = FALSE;
  config->proto_redir =
    CURLPROTO_ALL & ~(CURLPROTO_FILE|CURLPROTO_SCP); /* not FILE or SCP */
  config->proto_redir_present = FALSE;

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
    parseconfig(NULL, config); /* ignore possible failure */
  }

  if((argc < 2)  && !config->url_list) {
    helpf(config->errors, NULL);
    return CURLE_FAILED_INIT;
  }

  /* Parse options */
  for(i = 1; i < argc; i++) {
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
          int retval = CURLE_OK;
          if(res != PARAM_HELP_REQUESTED) {
            const char *reason = param2text(res);
            helpf(config->errors, "option %s: %s\n", origopt, reason);
            retval = CURLE_FAILED_INIT;
          }
          clean_getout(config);
          return retval;
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
    helpf(config->errors, "no URL specified!\n");
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

  /* On WIN32 we can't set the path to curl-ca-bundle.crt
   * at compile time. So we look here for the file in two ways:
   * 1: look at the environment variable CURL_CA_BUNDLE for a path
   * 2: if #1 isn't found, use the windows API function SearchPath()
   *    to find it along the app's path (includes app's dir and CWD)
   *
   * We support the environment variable thing for non-Windows platforms
   * too. Just for the sake of it.
   */
  if(!config->cacert &&
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
#ifdef WIN32
    else
      FindWin32CACert(config, "curl-ca-bundle.crt");
#endif
  }

  if(config->postfields) {
    if(config->use_httpget) {
      /* Use the postfields data for a http get */
      httpgetfields = strdup(config->postfields);
      free(config->postfields);
      config->postfields = NULL;
      if(SetHTTPrequest(config,
                        (config->no_body?HTTPREQ_HEAD:HTTPREQ_GET),
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

  /* This is the first entry added to easycode and it initializes the slist */
  easycode = curl_slist_append(easycode, "CURL *hnd = curl_easy_init();");
  if(!easycode) {
    clean_getout(config);
    res = CURLE_OUT_OF_MEMORY;
    goto quit_curl;
  }

  if(config->list_engines) {
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
    }
    else
      heads.stream=stdout;
    heads.config = config;
  }

  /* loop through the list of given URLs */
  while(urlnode) {
    int up; /* upload file counter within a single upload glob */
    char *dourl;
    char *url;
    char *infiles; /* might be a glob pattern */
    char *outfiles=NULL;

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
    if(urlnode->outfile) {
      outfiles = strdup(urlnode->outfile);
      if(!outfiles) {
        clean_getout(config);
        break;
      }
    }

    infiles = urlnode->infile;

    if(!config->globoff && infiles) {
      /* Unless explicitly shut off */
      res = glob_url(&inglob, infiles, &infilenum,
                     config->showerror?config->errors:NULL);
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
          ((uploadfile = inglob?
           glob_next_url(inglob):
           (!up?strdup(infiles):NULL)) != NULL);
        up++) {
      int separator = 0;
      long retry_numretries;
      uploadfilesize=-1;

      if(!config->globoff) {
        /* Unless explicitly shut off, we expand '{...}' and '[...]'
           expressions and return total number of URLs in pattern set */
        res = glob_url(&urls, dourl, &urlnum,
                       config->showerror?config->errors:NULL);
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
          ((url = urls?glob_next_url(urls):(i?NULL:strdup(url))) != NULL);
          i++) {
        /* NOTE: In the condition expression in the for() statement above, the
           'url' variable is only ever strdup()ed if(i == 0) and thus never
           when this loops later on. Further down in this function we call
           free(url) and then the code loops. Static code parsers may thus get
           tricked into believing that we have a potential access-after-free
           here.  I can however not spot any such case. */

        int infd = STDIN_FILENO;
        bool infdopen;
        char *outfile;
        struct timeval retrystart;
        outfile = outfiles?strdup(outfiles):NULL;

        if((urlnode->flags&GETOUT_USEREMOTE) ||
           (outfile && !curlx_strequal("-", outfile)) ) {

          /*
           * We have specified a file name to store the result in, or we have
           * decided we want to use the remote file name.
           */

          if(!outfile) {
            /* extract the file name from the URL */
            outfile = get_url_file_name(url);
            if((!outfile || !*outfile) && !config->content_disposition) {
              helpf(config->errors, "Remote file name has no length!\n");
              res = CURLE_WRITE_ERROR;
              free(url);
              break;
            }
#if defined(MSDOS) || defined(WIN32)
            /* For DOS and WIN32, we do some major replacing of
               bad characters in the file name before using it */
            outfile = sanitize_dos_name(outfile);
            if(!outfile) {
              res = CURLE_OUT_OF_MEMORY;
              break;
            }
#endif /* MSDOS || WIN32 */
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
             (-1 == create_dir_hierarchy(outfile, config->errors))) {
            free(url);
            res = CURLE_WRITE_ERROR;
            break;
          }

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
            if(!outs.stream) {
              helpf(config->errors, "Can't open '%s'!\n", outfile);
              free(url);
              res = CURLE_WRITE_ERROR;
              break;
            }
          }
          else {
            outs.stream = NULL; /* open when needed */
            outs.bytes = 0;     /* reset byte counter */
          }
        }
        infdopen=FALSE;
        if(uploadfile && !stdin_upload(uploadfile)) {
          /*
           * We have specified a file to upload and it isn't "-".
           */
          struct_stat fileinfo;

          url = add_file_name_to_url(curl, url, uploadfile);
          if(!url) {
            helpf(config->errors, "out of memory\n");
            res = CURLE_OUT_OF_MEMORY;
            break;
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

          infd= open(uploadfile, O_RDONLY | O_BINARY);
          if((infd == -1) || fstat(infd, &fileinfo)) {
            helpf(config->errors, "Can't open '%s'!\n", uploadfile);
            if(infd != -1)
              close(infd);

            /* Free the list of remaining URLs and globbed upload files
             * to force curl to exit immediately
             */
            if(urls) {
              glob_cleanup(urls);
              urls = NULL;
            }
            if(inglob) {
              glob_cleanup(inglob);
              inglob = NULL;
            }

            res = CURLE_READ_ERROR;
            goto quit_urls;
          }
          infdopen=TRUE;

          /* we ignore file size for char/block devices, sockets, etc. */
          if(S_ISREG(fileinfo.st_mode))
            uploadfilesize=fileinfo.st_size;

        }
        else if(uploadfile && stdin_upload(uploadfile)) {
          /* count to see if there are more than one auth bit set
             in the authtype field */
          int authbits = 0;
          int bitcheck = 0;
          while(bitcheck < 32) {
            if(config->authtype & (1 << bitcheck++)) {
              authbits++;
              if(authbits > 1) {
                /* more than one, we're done! */
                break;
              }
            }
          }

          /*
           * If the user has also selected --anyauth or --proxy-anyauth
           * we should warn him/her.
           */
          if(config->proxyanyauth || (authbits>1)) {
            warnf(config,
                  "Using --anyauth or --proxy-anyauth with upload from stdin"
                  " involves a big risk of it not working. Use a temporary"
                  " file or a fixed auth type instead!\n");
          }

          SET_BINMODE(stdin);
          infd = STDIN_FILENO;
          if(curlx_strequal(uploadfile, ".")) {
            if(curlx_nonblock((curl_socket_t)infd, TRUE) < 0)
              warnf(config,
                    "fcntl failed on fd=%d: %s\n", infd, strerror(errno));
          }
        }

        if(uploadfile && config->resume_from_current)
          config->resume_from = -1; /* -1 will then force get-it-yourself */

        if(output_expected(url, uploadfile)
           && outs.stream && isatty(fileno(outs.stream)))
          /* we send the output to a tty, therefore we switch off the progress
             meter */
          config->noprogress = config->isatty = TRUE;

        if(urlnum > 1 && !(config->mute)) {
          fprintf(config->errors, "\n[%d/%d]: %s --> %s\n",
                  i+1, urlnum, url, outfile ? outfile : "<stdout>");
          if(separator)
            printf("%s%s\n", CURLseparator, url);
        }
        if(httpgetfields) {
          char *urlbuffer;
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
          urlbuffer = malloc(strlen(url) + strlen(httpgetfields) + 3);
          if(!urlbuffer) {
            helpf(config->errors, "out of memory\n");

            /* Free the list of remaining URLs and globbed upload files
             * to force curl to exit immediately
             */
            if(urls) {
              glob_cleanup(urls);
              urls = NULL;
            }
            if(inglob) {
              glob_cleanup(inglob);
              inglob = NULL;
            }

            res = CURLE_OUT_OF_MEMORY;
            goto quit_urls;
          }
          if(pc)
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

        if((!outfile || !strcmp(outfile, "-")) && !config->use_ascii) {
          /* We get the output to stdout and we have not got the ASCII/text
             flag, then set stdout to be binary */
          SET_BINMODE(stdout);
        }

        if(1 == config->tcp_nodelay)
          my_setopt(curl, CURLOPT_TCP_NODELAY, 1);

        /* where to store */
        my_setopt(curl, CURLOPT_WRITEDATA, &outs);
        /* what call to write */
        my_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);

        /* for uploads */
        input.fd = infd;
        input.config = config;
        my_setopt(curl, CURLOPT_READDATA, &input);
        /* what call to read */
        if((outfile && !curlx_strequal("-", outfile)) ||
           !checkprefix("telnet:", url))
          my_setopt(curl, CURLOPT_READFUNCTION, my_fread);

        /* in 7.18.0, the CURLOPT_SEEKFUNCTION/DATA pair is taking over what
           CURLOPT_IOCTLFUNCTION/DATA pair previously provided for seeking */
        my_setopt(curl, CURLOPT_SEEKDATA, &input);
        my_setopt(curl, CURLOPT_SEEKFUNCTION, my_seek);

        if(config->recvpersecond)
          /* tell libcurl to use a smaller sized buffer as it allows us to
             make better sleeps! 7.9.9 stuff! */
          my_setopt(curl, CURLOPT_BUFFERSIZE, config->recvpersecond);

        /* size of uploaded file: */
        if(uploadfilesize != -1)
          my_setopt(curl, CURLOPT_INFILESIZE_LARGE, uploadfilesize);
        my_setopt_str(curl, CURLOPT_URL, url);     /* what to fetch */
        my_setopt_str(curl, CURLOPT_PROXY, config->proxy); /* proxy to use */
        if(config->proxy)
          my_setopt(curl, CURLOPT_PROXYTYPE, config->proxyver);
        my_setopt(curl, CURLOPT_NOPROGRESS, config->noprogress);
        if(config->no_body) {
          my_setopt(curl, CURLOPT_NOBODY, 1);
          my_setopt(curl, CURLOPT_HEADER, 1);
        }
        else
          my_setopt(curl, CURLOPT_HEADER, config->include_headers);

        my_setopt(curl, CURLOPT_FAILONERROR, config->failonerror);
        my_setopt(curl, CURLOPT_UPLOAD, uploadfile?TRUE:FALSE);
        my_setopt(curl, CURLOPT_DIRLISTONLY, config->dirlistonly);
        my_setopt(curl, CURLOPT_APPEND, config->ftp_append);

        if(config->netrc_opt)
          my_setopt(curl, CURLOPT_NETRC, CURL_NETRC_OPTIONAL);
        else if(config->netrc)
          my_setopt(curl, CURLOPT_NETRC, CURL_NETRC_REQUIRED);
        else
          my_setopt(curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);

        my_setopt(curl, CURLOPT_FOLLOWLOCATION, config->followlocation);
        my_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, config->unrestricted_auth);
        my_setopt(curl, CURLOPT_TRANSFERTEXT, config->use_ascii);
        my_setopt_str(curl, CURLOPT_USERPWD, config->userpwd);
        my_setopt_str(curl, CURLOPT_PROXYUSERPWD, config->proxyuserpwd);
        my_setopt(curl, CURLOPT_NOPROXY, config->noproxy);
        my_setopt_str(curl, CURLOPT_RANGE, config->range);
        my_setopt(curl, CURLOPT_ERRORBUFFER, errorbuffer);
        my_setopt(curl, CURLOPT_TIMEOUT, config->timeout);

        switch(config->httpreq) {
        case HTTPREQ_SIMPLEPOST:
          my_setopt_str(curl, CURLOPT_POSTFIELDS, config->postfields);
          my_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, config->postfieldsize);
          break;
        case HTTPREQ_POST:
          my_setopt(curl, CURLOPT_HTTPPOST, config->httppost);
          break;
        default:
          break;
        }
        my_setopt_str(curl, CURLOPT_REFERER, config->referer);
        my_setopt(curl, CURLOPT_AUTOREFERER, config->autoreferer);
        my_setopt_str(curl, CURLOPT_USERAGENT, config->useragent);
        my_setopt_str(curl, CURLOPT_FTPPORT, config->ftpport);
        my_setopt(curl, CURLOPT_LOW_SPEED_LIMIT,
                  config->low_speed_limit);
        my_setopt(curl, CURLOPT_LOW_SPEED_TIME, config->low_speed_time);
        my_setopt(curl, CURLOPT_MAX_SEND_SPEED_LARGE,
                  config->sendpersecond);
        my_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE,
                  config->recvpersecond);
        my_setopt(curl, CURLOPT_RESUME_FROM_LARGE,
                  config->use_resume?config->resume_from:0);
        my_setopt_str(curl, CURLOPT_COOKIE, config->cookie);
        my_setopt(curl, CURLOPT_HTTPHEADER, config->headers);
        my_setopt(curl, CURLOPT_SSLCERT, config->cert);
        my_setopt_str(curl, CURLOPT_SSLCERTTYPE, config->cert_type);
        my_setopt(curl, CURLOPT_SSLKEY, config->key);
        my_setopt_str(curl, CURLOPT_SSLKEYTYPE, config->key_type);
        my_setopt_str(curl, CURLOPT_KEYPASSWD, config->key_passwd);

        /* SSH private key uses the same command-line option as SSL private
           key */
        my_setopt_str(curl, CURLOPT_SSH_PRIVATE_KEYFILE, config->key);
        my_setopt_str(curl, CURLOPT_SSH_PUBLIC_KEYFILE, config->pubkey);

        /* SSH host key md5 checking allows us to fail if we are
         * not talking to who we think we should
         */
        my_setopt_str(curl, CURLOPT_SSH_HOST_PUBLIC_KEY_MD5,
                      config->hostpubmd5);

        /* default to strict verifyhost */
        /* my_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2); */
        if(config->cacert || config->capath) {
          if(config->cacert)
            my_setopt_str(curl, CURLOPT_CAINFO, config->cacert);

          if(config->capath)
            my_setopt_str(curl, CURLOPT_CAPATH, config->capath);
          my_setopt(curl, CURLOPT_SSL_VERIFYPEER, TRUE);
        }
        if(config->crlfile)
          my_setopt_str(curl, CURLOPT_CRLFILE, config->crlfile);
        if(config->insecure_ok) {
          /* new stuff needed for libcurl 7.10 */
          my_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
          my_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        }
        else {
          char *home = homedir();
          char *file = aprintf("%s/%sssh/known_hosts", home, DOT_CHAR);
          if(home)
            free(home);

          if(file) {
            my_setopt_str(curl, CURLOPT_SSH_KNOWNHOSTS, file);
            curl_free(file);
          }
          else {
            /* Free the list of remaining URLs and globbed upload files
             * to force curl to exit immediately
             */
            if(urls) {
              glob_cleanup(urls);
              urls = NULL;
            }
            if(inglob) {
              glob_cleanup(inglob);
              inglob = NULL;
            }

            res = CURLE_OUT_OF_MEMORY;
            goto quit_urls;
          }
        }

        if(config->no_body || config->remote_time) {
          /* no body or use remote time */
          my_setopt(curl, CURLOPT_FILETIME, TRUE);
        }

        my_setopt(curl, CURLOPT_MAXREDIRS, config->maxredirs);
        my_setopt(curl, CURLOPT_CRLF, config->crlf);
        my_setopt(curl, CURLOPT_QUOTE, config->quote);
        my_setopt(curl, CURLOPT_POSTQUOTE, config->postquote);
        my_setopt(curl, CURLOPT_PREQUOTE, config->prequote);
        my_setopt(curl, CURLOPT_HEADERDATA,
                  config->headerfile?&heads:NULL);
        my_setopt_str(curl, CURLOPT_COOKIEFILE, config->cookiefile);
        /* cookie jar was added in 7.9 */
        if(config->cookiejar)
          my_setopt_str(curl, CURLOPT_COOKIEJAR, config->cookiejar);
        /* cookie session added in 7.9.7 */
        my_setopt(curl, CURLOPT_COOKIESESSION, config->cookiesession);

        my_setopt(curl, CURLOPT_SSLVERSION, config->ssl_version);
        my_setopt(curl, CURLOPT_TIMECONDITION, config->timecond);
        my_setopt(curl, CURLOPT_TIMEVALUE, config->condtime);
        my_setopt_str(curl, CURLOPT_CUSTOMREQUEST, config->customrequest);
        my_setopt(curl, CURLOPT_STDERR, config->errors);

        /* three new ones in libcurl 7.3: */
        my_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, config->proxytunnel);
        my_setopt_str(curl, CURLOPT_INTERFACE, config->iface);
        my_setopt_str(curl, CURLOPT_KRBLEVEL, config->krblevel);

        progressbarinit(&progressbar, config);
        if((config->progressmode == CURL_PROGRESS_BAR) &&
           !config->noprogress && !config->mute) {
          /* we want the alternative style, then we have to implement it
             ourselves! */
          my_setopt(curl, CURLOPT_PROGRESSFUNCTION, myprogress);
          my_setopt(curl, CURLOPT_PROGRESSDATA, &progressbar);
        }

        /* new in libcurl 7.6.2: */
        my_setopt(curl, CURLOPT_TELNETOPTIONS, config->telnet_options);

        /* new in libcurl 7.7: */
        my_setopt_str(curl, CURLOPT_RANDOM_FILE, config->random_file);
        my_setopt(curl, CURLOPT_EGDSOCKET, config->egd_file);
        my_setopt(curl, CURLOPT_CONNECTTIMEOUT, config->connecttimeout);

        if(config->cipher_list)
          my_setopt_str(curl, CURLOPT_SSL_CIPHER_LIST, config->cipher_list);

        if(config->httpversion)
          my_setopt(curl, CURLOPT_HTTP_VERSION, config->httpversion);

        /* new in libcurl 7.9.2: */
        if(config->disable_epsv)
          /* disable it */
          my_setopt(curl, CURLOPT_FTP_USE_EPSV, FALSE);

        /* new in libcurl 7.10.5 */
        if(config->disable_eprt)
          /* disable it */
          my_setopt(curl, CURLOPT_FTP_USE_EPRT, FALSE);

        /* new in libcurl 7.10.6 (default is Basic) */
        if(config->authtype)
          my_setopt(curl, CURLOPT_HTTPAUTH, config->authtype);

        if(config->tracetype != TRACE_NONE) {
          my_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
          my_setopt(curl, CURLOPT_DEBUGDATA, config);
          my_setopt(curl, CURLOPT_VERBOSE, TRUE);
        }

        res = CURLE_OK;

        /* new in curl ?? */
        if(config->engine) {
          res = my_setopt_str(curl, CURLOPT_SSLENGINE, config->engine);
          my_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1);
        }

        if(res != CURLE_OK)
          goto show_error;

        /* new in curl 7.10 */
        my_setopt_str(curl, CURLOPT_ENCODING,
                      (config->encoding) ? "" : NULL);

        /* new in curl 7.10.7, extended in 7.19.4 but this only sets 0 or 1 */
        my_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS,
                  config->ftp_create_dirs);
        if(config->proxyanyauth)
          my_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        else if(config->proxynegotiate)
          my_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_GSSNEGOTIATE);
        else if(config->proxyntlm)
          my_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);
        else if(config->proxydigest)
          my_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_DIGEST);
        else if(config->proxybasic)
          my_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);

        /* new in curl 7.10.8 */
        if(config->max_filesize)
          my_setopt(curl, CURLOPT_MAXFILESIZE_LARGE,
                    config->max_filesize);

        if(4 == config->ip_version)
          my_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        else if(6 == config->ip_version)
          my_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
        else
          my_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);

        /* new in curl 7.15.5 */
        if(config->ftp_ssl_reqd)
          my_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        /* new in curl 7.11.0 */
        else if(config->ftp_ssl)
          my_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);

        /* new in curl 7.16.0 */
        else if(config->ftp_ssl_control)
          my_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_CONTROL);

        /* new in curl 7.16.1 */
        if(config->ftp_ssl_ccc)
          my_setopt(curl, CURLOPT_FTP_SSL_CCC, config->ftp_ssl_ccc_mode);

        /* new in curl 7.11.1, modified in 7.15.2 */
        if(config->socksproxy) {
          my_setopt_str(curl, CURLOPT_PROXY, config->socksproxy);
          my_setopt(curl, CURLOPT_PROXYTYPE, config->socksver);
        }

#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
        /* new in curl 7.19.4 */
        if(config->socks5_gssapi_service)
          my_setopt_str(curl, CURLOPT_SOCKS5_GSSAPI_SERVICE,
                        config->socks5_gssapi_service);

        /* new in curl 7.19.4 */
        if(config->socks5_gssapi_nec)
          my_setopt_str(curl, CURLOPT_SOCKS5_GSSAPI_NEC,
                        config->socks5_gssapi_nec);
#endif
        /* curl 7.13.0 */
        my_setopt_str(curl, CURLOPT_FTP_ACCOUNT, config->ftp_account);

        my_setopt(curl, CURLOPT_IGNORE_CONTENT_LENGTH, config->ignorecl);

        /* curl 7.14.2 */
        my_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, config->ftp_skip_ip);

        /* curl 7.15.1 */
        my_setopt(curl, CURLOPT_FTP_FILEMETHOD, config->ftp_filemethod);

        /* curl 7.15.2 */
        if(config->localport) {
          my_setopt(curl, CURLOPT_LOCALPORT, config->localport);
          my_setopt_str(curl, CURLOPT_LOCALPORTRANGE,
                        config->localportrange);
        }

        /* curl 7.15.5 */
        my_setopt_str(curl, CURLOPT_FTP_ALTERNATIVE_TO_USER,
                      config->ftp_alternative_to_user);

        /* curl 7.16.0 */
        if(config->disable_sessionid)
          my_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE,
                    !config->disable_sessionid);

        /* curl 7.16.2 */
        if(config->raw) {
          my_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, FALSE);
          my_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, FALSE);
        }

        /* curl 7.17.1 */
        if(!config->nokeepalive) {
          my_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockoptcallback);
          my_setopt(curl, CURLOPT_SOCKOPTDATA, config);
        }

        /* curl 7.19.1 (the 301 version existed in 7.18.2) */
        my_setopt(curl, CURLOPT_POSTREDIR, config->post301 |
                  (config->post302 ? CURL_REDIR_POST_302 : FALSE));

        /* curl 7.20.0 */
        if(config->tftp_blksize)
          my_setopt(curl, CURLOPT_TFTP_BLKSIZE, config->tftp_blksize);

        if(config->mail_from)
          my_setopt_str(curl, CURLOPT_MAIL_FROM, config->mail_from);

        if(config->mail_rcpt)
          my_setopt(curl, CURLOPT_MAIL_RCPT, config->mail_rcpt);

        /* curl 7.20.x */
        if(config->ftp_pret)
          my_setopt(curl, CURLOPT_FTP_USE_PRET, TRUE);

        if(config->proto_present)
          my_setopt(curl, CURLOPT_PROTOCOLS, config->proto);
        if(config->proto_redir_present)
          my_setopt(curl, CURLOPT_REDIR_PROTOCOLS, config->proto_redir);

        if((urlnode->flags & GETOUT_USEREMOTE)
           && config->content_disposition) {
          my_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
          my_setopt(curl, CURLOPT_HEADERDATA, &outs);
        }

        if(config->resolve)
          /* new in 7.21.3 */
          my_setopt(curl, CURLOPT_RESOLVE, config->resolve);

        retry_numretries = config->req_retry;

        retrystart = cutil_tvnow();

        for(;;) {
          res = curl_easy_perform(curl);
          if(!curl_slist_append(easycode, "ret = curl_easy_perform(hnd);")) {
            res = CURLE_OUT_OF_MEMORY;
            break;
          }

          if(config->content_disposition && outs.stream && !config->mute &&
             outs.filename)
            printf("curl: Saved to filename '%s'\n", outs.filename);

          /* if retry-max-time is non-zero, make sure we haven't exceeded the
             time */
          if(retry_numretries &&
             (!config->retry_maxtime ||
              (cutil_tvdiff(cutil_tvnow(), retrystart)<
               config->retry_maxtime*1000)) ) {
            enum {
              RETRY_NO,
              RETRY_TIMEOUT,
              RETRY_HTTP,
              RETRY_FTP,
              RETRY_LAST /* not used */
            } retry = RETRY_NO;
            long response;
            if(CURLE_OPERATION_TIMEDOUT == res)
              /* retry timeout always */
              retry = RETRY_TIMEOUT;
            else if((CURLE_OK == res) ||
                    (config->failonerror &&
                     (CURLE_HTTP_RETURNED_ERROR == res))) {
              /* If it returned OK. _or_ failonerror was enabled and it
                 returned due to such an error, check for HTTP transient
                 errors to retry on. */
              char *this_url=NULL;
              curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &this_url);
              if(this_url &&
                 checkprefix("http", this_url)) {
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
            else if(res) {
              curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);

              if(response/100 == 4)
                /*
                 * This is typically when the FTP server only allows a certain
                 * amount of users and we are not one of them.  All 4xx codes
                 * are transient.
                 */
                retry = RETRY_FTP;
            }

            if(retry) {
              static const char * const m[]={
                NULL, "timeout", "HTTP error", "FTP error"
              };
              warnf(config, "Transient problem: %s "
                    "Will retry in %ld seconds. "
                    "%ld retries left.\n",
                    m[retry], retry_sleep/1000, retry_numretries);

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
                if(!config->mute)
                  fprintf(config->errors, "Throwing away %"
                          CURL_FORMAT_CURL_OFF_T " bytes\n",
                          outs.bytes);
                fflush(outs.stream);
                /* truncate file at the position where we started appending */
#ifdef HAVE_FTRUNCATE
                if(ftruncate( fileno(outs.stream), outs.init)) {
                  /* when truncate fails, we can't just append as then we'll
                     create something strange, bail out */
                  if(!config->mute)
                    fprintf(config->errors,
                            "failed to truncate, exiting\n");
                  break;
                }
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

        }

        if((config->progressmode == CURL_PROGRESS_BAR) &&
           progressbar.calls)
          /* if the custom progress bar has been displayed, we output a
             newline here */
          fputs("\n", progressbar.out);

        if(config->writeout)
          ourWriteOut(curl, config->writeout);
#ifdef USE_ENVIRONMENT
        if(config->writeenv)
          ourWriteEnv(curl);
#endif

        show_error:

#ifdef __VMS
        if(is_vms_shell()) {
          /* VMS DCL shell behavior */
          if(!config->showerror) {
            vms_show = VMSSTS_HIDE;
          }
        }
        else
#endif
        {
          if((res!=CURLE_OK) && config->showerror) {
            fprintf(config->errors, "curl: (%d) %s\n", res,
                    errorbuffer[0]? errorbuffer:
                    curl_easy_strerror((CURLcode)res));
            if(CURLE_SSL_CACERT == res) {
#define CURL_CA_CERT_ERRORMSG1                                          \
              "More details here: http://curl.haxx.se/docs/sslcerts.html\n\n" \
                "curl performs SSL certificate verification by default, using a \"bundle\"\n" \
                " of Certificate Authority (CA) public keys (CA certs). If the default\n" \
                " bundle file isn't adequate, you can specify an alternate file\n" \
                " using the --cacert option.\n"

#define CURL_CA_CERT_ERRORMSG2                                          \
              "If this HTTPS server uses a certificate signed by a CA represented in\n" \
                " the bundle, the certificate verification probably failed due to a\n" \
                " problem with the certificate (it might be expired, or the name might\n" \
                " not match the domain name in the URL).\n"             \
                "If you'd like to turn off curl's verification of the certificate, use\n" \
                " the -k (or --insecure) option.\n"

              fprintf(config->errors, "%s%s",
                      CURL_CA_CERT_ERRORMSG1,
                      CURL_CA_CERT_ERRORMSG2 );
            }
          }
        }
        if(outfile && !curlx_strequal(outfile, "-") && outs.stream) {
          int rc;

          if(config->xattr) {
            rc = fwrite_xattr(curl, fileno(outs.stream) );
            if(rc)
              warnf(config, "Error setting extended attributes: %s\n",
                    strerror(errno) );
          }

          rc = fclose(outs.stream);
          if(!res && rc) {
            /* something went wrong in the writing process */
            res = CURLE_WRITE_ERROR;
            fprintf(config->errors, "(%d) Failed writing body\n", res);
          }
        }

#ifdef HAVE_UTIME
        /* Important that we set the time _after_ the file has been
           closed, as is done above here */
        if(config->remote_time && outs.filename) {
          /* ask libcurl if we got a time. Pretty please */
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
#ifdef __AMIGA__
        /* Set the url as comment for the file. (up to 80 chars are allowed)
         */
        if( strlen(url) > 78 )
          url[79] = '\0';

        SetComment( outs.filename, url);
#endif

        quit_urls:
        if(url)
          free(url);

        if(outfile)
          free(outfile);

        if(infdopen)
          close(infd);

      } /* loop to the next URL */

      if(urls) {
        /* cleanup memory used for URL globbing patterns */
        glob_cleanup(urls);
        urls = NULL;
      }

      if(uploadfile)
        free(uploadfile);

    } /* loop to the next globbed upload file */

    if(inglob) {
      glob_cleanup(inglob);
      inglob = NULL;
    }

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
  if(httpgetfields)
    free(httpgetfields);

  if(config->engine)
    free(config->engine);

  /* cleanup the curl handle! */
  curl_easy_cleanup(curl);
  config->easy = NULL; /* cleanup now */
  if(easycode)
    curl_slist_append(easycode, "curl_easy_cleanup(hnd);");

  if(heads.stream && (heads.stream != stdout))
    fclose(heads.stream);

  if(allocuseragent)
    free(config->useragent);

  if(config->trace_fopened && config->trace_stream)
    fclose(config->trace_stream);

  /* Dump the libcurl code if previously enabled.
     NOTE: that this function relies on config->errors amongst other things
     so not everything can be closed and cleaned before this is called */
  dumpeasycode(config);

  if(config->errors_fopened)
    fclose(config->errors);

  main_free(); /* cleanup */

  return res;
}