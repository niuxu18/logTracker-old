ParameterError getparameter(char *flag,    /* f or -long-flag */
                            char *nextarg, /* NULL if unset */
                            bool *usedarg, /* set to TRUE if the arg
                                              has been used */
                            struct GlobalConfig *global,
                            struct OperationConfig *config)
{
  char letter;
  char subletter = '\0'; /* subletters can only occur on long options */
  int rc;
  const char *parse = NULL;
  unsigned int j;
  time_t now;
  int hit = -1;
  bool longopt = FALSE;
  bool singleopt = FALSE; /* when true means '-o foo' used '-ofoo' */
  ParameterError err;
  bool toggle = TRUE; /* how to switch boolean options, on or off. Controlled
                         by using --OPTION or --no-OPTION */


  if(('-' != flag[0]) ||
     (('-' == flag[0]) && ('-' == flag[1]))) {
    /* this should be a long name */
    char *word = ('-' == flag[0]) ? flag+2 : flag;
    size_t fnam = strlen(word);
    int numhits = 0;

    if(!strncmp(word, "no-", 3)) {
      /* disable this option but ignore the "no-" part when looking for it */
      word += 3;
      toggle = FALSE;
    }

    for(j = 0; j < sizeof(aliases)/sizeof(aliases[0]); j++) {
      if(curlx_strnequal(aliases[j].lname, word, fnam)) {
        longopt = TRUE;
        numhits++;
        if(curlx_raw_equal(aliases[j].lname, word)) {
          parse = aliases[j].letter;
          hit = j;
          numhits = 1; /* a single unique hit */
          break;
        }
        parse = aliases[j].letter;
        hit = j;
      }
    }
    if(numhits > 1) {
      /* this is at least the second match! */
      return PARAM_OPTION_AMBIGUOUS;
    }
    if(hit < 0) {
      return PARAM_OPTION_UNKNOWN;
    }
  }
  else {
    flag++; /* prefixed with one dash, pass it */
    hit = -1;
    parse = flag;
  }

  do {
    /* we can loop here if we have multiple single-letters */

    if(!longopt) {
      letter = (char)*parse;
      subletter='\0';
    }
    else {
      letter = parse[0];
      subletter = parse[1];
    }
    *usedarg = FALSE; /* default is that we don't use the arg */

    if(hit < 0) {
      for(j = 0; j < sizeof(aliases)/sizeof(aliases[0]); j++) {
        if(letter == aliases[j].letter[0]) {
          hit = j;
          break;
        }
      }
      if(hit < 0) {
        return PARAM_OPTION_UNKNOWN;
      }
    }

    if(aliases[hit].extraparam) {
      /* this option requires an extra parameter */
      if(!longopt && parse[1]) {
        nextarg = (char *)&parse[1]; /* this is the actual extra parameter */
        singleopt = TRUE;   /* don't loop anymore after this */
      }
      else if(!nextarg)
        return PARAM_REQUIRES_PARAMETER;
      else
        *usedarg = TRUE; /* mark it as used */
    }

    switch(letter) {
    case '*': /* options without a short option */
      switch(subletter) {
      case '4': /* --dns-ipv4-addr */
        /* addr in dot notation */
        GetStr(&config->dns_ipv4_addr, nextarg);
        break;
      case '6': /* --dns-ipv6-addr */
        /* addr in dot notation */
        GetStr(&config->dns_ipv6_addr, nextarg);
        break;
      case 'a': /* random-file */
        GetStr(&config->random_file, nextarg);
        break;
      case 'b': /* egd-file */
        GetStr(&config->egd_file, nextarg);
        break;
      case 'B': /* XOAUTH2 Bearer */
        GetStr(&config->xoauth2_bearer, nextarg);
        break;
      case 'c': /* connect-timeout */
        err = str2udouble(&config->connecttimeout, nextarg);
        if(err)
          return err;
        break;
      case 'd': /* ciphers */
        GetStr(&config->cipher_list, nextarg);
        break;
      case 'D': /* --dns-interface */
        /* interface name */
        GetStr(&config->dns_interface, nextarg);
        break;
      case 'e': /* --disable-epsv */
        config->disable_epsv = toggle;
        break;
      case 'E': /* --epsv */
        config->disable_epsv = (!toggle)?TRUE:FALSE;
        break;
#ifdef USE_ENVIRONMENT
      case 'f':
        config->writeenv = toggle;
        break;
#endif
      case 'F': /* --dns-servers */
        /* IP addrs of DNS servers */
        GetStr(&config->dns_servers, nextarg);
        break;
      case 'g': /* --trace */
        GetStr(&global->trace_dump, nextarg);
        if(global->tracetype && (global->tracetype != TRACE_BIN))
          warnf(config, "--trace overrides an earlier trace/verbose option\n");
        global->tracetype = TRACE_BIN;
        break;
      case 'G': /* --npn */
        config->nonpn = (!toggle)?TRUE:FALSE;
        break;
      case 'h': /* --trace-ascii */
        GetStr(&global->trace_dump, nextarg);
        if(global->tracetype && (global->tracetype != TRACE_ASCII))
          warnf(config,
                "--trace-ascii overrides an earlier trace/verbose option\n");
        global->tracetype = TRACE_ASCII;
        break;
      case 'H': /* --alpn */
        config->noalpn = (!toggle)?TRUE:FALSE;
        break;
      case 'i': /* --limit-rate */
      {
        /* We support G, M, K too */
        char *unit;
        curl_off_t value = curlx_strtoofft(nextarg, &unit, 0);

        if(!*unit)
          unit = (char *)"b";
        else if(strlen(unit) > 1)
          unit = (char *)"w"; /* unsupported */

        switch(*unit) {
        case 'G':
        case 'g':
          value *= 1024*1024*1024;
          break;
        case 'M':
        case 'm':
          value *= 1024*1024;
          break;
        case 'K':
        case 'k':
          value *= 1024;
          break;
        case 'b':
        case 'B':
          /* for plain bytes, leave as-is */
          break;
        default:
          warnf(config, "unsupported rate unit. Use G, M, K or B!\n");
          return PARAM_BAD_USE;
        }
        config->recvpersecond = value;
        config->sendpersecond = value;
      }
      break;

      case 'j': /* --compressed */
        if(toggle && !(curlinfo->features & CURL_VERSION_LIBZ))
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        config->encoding = toggle;
        break;

      case 'J': /* --tr-encoding */
        config->tr_encoding = toggle;
        break;

      case 'k': /* --digest */
        if(toggle)
          config->authtype |= CURLAUTH_DIGEST;
        else
          config->authtype &= ~CURLAUTH_DIGEST;
        break;

      case 'l': /* --negotiate */
        if(toggle) {
          if(curlinfo->features & CURL_VERSION_SPNEGO)
            config->authtype |= CURLAUTH_NEGOTIATE;
          else
            return PARAM_LIBCURL_DOESNT_SUPPORT;
        }
        else
          config->authtype &= ~CURLAUTH_NEGOTIATE;
        break;

      case 'm': /* --ntlm */
        if(toggle) {
          if(curlinfo->features & CURL_VERSION_NTLM)
            config->authtype |= CURLAUTH_NTLM;
          else
            return PARAM_LIBCURL_DOESNT_SUPPORT;
        }
        else
          config->authtype &= ~CURLAUTH_NTLM;
        break;

      case 'M': /* --ntlm-wb */
        if(toggle) {
          if(curlinfo->features & CURL_VERSION_NTLM_WB)
            config->authtype |= CURLAUTH_NTLM_WB;
          else
            return PARAM_LIBCURL_DOESNT_SUPPORT;
        }
        else
          config->authtype &= ~CURLAUTH_NTLM_WB;
        break;

      case 'n': /* --basic for completeness */
        if(toggle)
          config->authtype |= CURLAUTH_BASIC;
        else
          config->authtype &= ~CURLAUTH_BASIC;
        break;

      case 'o': /* --anyauth, let libcurl pick it */
        if(toggle)
          config->authtype = CURLAUTH_ANY;
        /* --no-anyauth simply doesn't touch it */
        break;

#ifdef USE_WATT32
      case 'p': /* --wdebug */
        dbug_init();
        break;
#endif
      case 'q': /* --ftp-create-dirs */
        config->ftp_create_dirs = toggle;
        break;

      case 'r': /* --create-dirs */
        config->create_dirs = toggle;
        break;

      case 's': /* --max-redirs */
        /* specified max no of redirects (http(s)), this accepts -1 as a
           special condition */
        err = str2num(&config->maxredirs, nextarg);
        if(err)
          return err;
        if(config->maxredirs < -1)
          return PARAM_BAD_NUMERIC;
        break;

      case 't': /* --proxy-ntlm */
        if(curlinfo->features & CURL_VERSION_NTLM)
          config->proxyntlm = toggle;
        else
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        break;

      case 'u': /* --crlf */
        /* LF -> CRLF conversion? */
        config->crlf = toggle;
        break;

      case 'v': /* --stderr */
        if(strcmp(nextarg, "-")) {
          FILE *newfile = fopen(nextarg, "wt");
          if(!newfile)
            warnf(config, "Failed to open %s!\n", nextarg);
          else {
            if(global->errors_fopened)
              fclose(global->errors);
            global->errors = newfile;
            global->errors_fopened = TRUE;
          }
        }
        else
          global->errors = stdout;
        break;
      case 'w': /* --interface */
        /* interface */
        GetStr(&config->iface, nextarg);
        break;
      case 'x': /* --krb */
        /* kerberos level string */
        if(curlinfo->features & CURL_VERSION_KERBEROS4)
          GetStr(&config->krblevel, nextarg);
        else
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        break;
      case 'y': /* --max-filesize */
        err = str2offset(&config->max_filesize, nextarg);
        if(err)
          return err;
        break;
      case 'z': /* --disable-eprt */
        config->disable_eprt = toggle;
        break;
      case 'Z': /* --eprt */
        config->disable_eprt = (!toggle)?TRUE:FALSE;
        break;

      default: /* the URL! */
      {
        struct getout *url;
        if(config->url_get || ((config->url_get = config->url_list) != NULL)) {
          /* there's a node here, if it already is filled-in continue to find
             an "empty" node */
          while(config->url_get && (config->url_get->flags & GETOUT_URL))
            config->url_get = config->url_get->next;
        }

        /* now there might or might not be an available node to fill in! */

        if(config->url_get)
          /* existing node */
          url = config->url_get;
        else
          /* there was no free node, create one! */
          url = new_getout(config);

        if(!url)
          return PARAM_NO_MEM;
        else {
          /* fill in the URL */
          GetStr(&url->url, nextarg);
          url->flags |= GETOUT_URL;
        }
      }
      }
      break;
    case '$': /* more options without a short option */
      switch(subletter) {
      case 'a': /* --ftp-ssl */
        if(toggle && !(curlinfo->features & CURL_VERSION_SSL))
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        config->ftp_ssl = toggle;
        break;
      case 'b': /* --ftp-pasv */
        Curl_safefree(config->ftpport);
        break;
      case 'c': /* --socks5 specifies a socks5 proxy to use, and resolves
                   the name locally and passes on the resolved address */
        GetStr(&config->socksproxy, nextarg);
        config->socksver = CURLPROXY_SOCKS5;
        break;
      case 't': /* --socks4 specifies a socks4 proxy to use */
        GetStr(&config->socksproxy, nextarg);
        config->socksver = CURLPROXY_SOCKS4;
        break;
      case 'T': /* --socks4a specifies a socks4a proxy to use */
        GetStr(&config->socksproxy, nextarg);
        config->socksver = CURLPROXY_SOCKS4A;
        break;
      case '2': /* --socks5-hostname specifies a socks5 proxy and enables name
                   resolving with the proxy */
        GetStr(&config->socksproxy, nextarg);
        config->socksver = CURLPROXY_SOCKS5_HOSTNAME;
        break;
      case 'd': /* --tcp-nodelay option */
        config->tcp_nodelay = toggle;
        break;
      case 'e': /* --proxy-digest */
        config->proxydigest = toggle;
        break;
      case 'f': /* --proxy-basic */
        config->proxybasic = toggle;
        break;
      case 'g': /* --retry */
        err = str2unum(&config->req_retry, nextarg);
        if(err)
          return err;
        break;
      case 'h': /* --retry-delay */
        err = str2unum(&config->retry_delay, nextarg);
        if(err)
          return err;
        break;
      case 'i': /* --retry-max-time */
        err = str2unum(&config->retry_maxtime, nextarg);
        if(err)
          return err;
        break;

      case 'k': /* --proxy-negotiate */
        if(curlinfo->features & CURL_VERSION_SPNEGO)
          config->proxynegotiate = toggle;
        else
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        break;

      case 'm': /* --ftp-account */
        GetStr(&config->ftp_account, nextarg);
        break;
      case 'n': /* --proxy-anyauth */
        config->proxyanyauth = toggle;
        break;
      case 'o': /* --trace-time */
        global->tracetime = toggle;
        break;
      case 'p': /* --ignore-content-length */
        config->ignorecl = toggle;
        break;
      case 'q': /* --ftp-skip-pasv-ip */
        config->ftp_skip_ip = toggle;
        break;
      case 'r': /* --ftp-method (undocumented at this point) */
        config->ftp_filemethod = ftpfilemethod(config, nextarg);
        break;
      case 's': /* --local-port */
        rc = sscanf(nextarg, "%d - %d",
                    &config->localport,
                    &config->localportrange);
        if(!rc)
          return PARAM_BAD_USE;
        else if(rc == 1)
          config->localportrange = 1; /* default number of ports to try */
        else {
          config->localportrange -= config->localport;
          if(config->localportrange < 1) {
            warnf(config, "bad range input\n");
            return PARAM_BAD_USE;
          }
        }
        break;
      case 'u': /* --ftp-alternative-to-user */
        GetStr(&config->ftp_alternative_to_user, nextarg);
        break;
      case 'v': /* --ftp-ssl-reqd */
        if(toggle && !(curlinfo->features & CURL_VERSION_SSL))
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        config->ftp_ssl_reqd = toggle;
        break;
      case 'w': /* --no-sessionid */
        config->disable_sessionid = (!toggle)?TRUE:FALSE;
        break;
      case 'x': /* --ftp-ssl-control */
        if(toggle && !(curlinfo->features & CURL_VERSION_SSL))
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        config->ftp_ssl_control = toggle;
        break;
      case 'y': /* --ftp-ssl-ccc */
        config->ftp_ssl_ccc = toggle;
        if(!config->ftp_ssl_ccc_mode)
          config->ftp_ssl_ccc_mode = CURLFTPSSL_CCC_PASSIVE;
        break;
      case 'j': /* --ftp-ssl-ccc-mode */
        config->ftp_ssl_ccc = TRUE;
        config->ftp_ssl_ccc_mode = ftpcccmethod(config, nextarg);
        break;
      case 'z': /* --libcurl */
#ifdef CURL_DISABLE_LIBCURL_OPTION
        warnf(config,
              "--libcurl option was disabled at build-time!\n");
        return PARAM_OPTION_UNKNOWN;
#else
        GetStr(&global->libcurl, nextarg);
        break;
#endif
      case '#': /* --raw */
        config->raw = toggle;
        break;
      case '0': /* --post301 */
        config->post301 = toggle;
        break;
      case '1': /* --no-keepalive */
        config->nokeepalive = (!toggle)?TRUE:FALSE;
        break;
      case '3': /* --keepalive-time */
        err = str2unum(&config->alivetime, nextarg);
        if(err)
          return err;
        break;
      case '4': /* --post302 */
        config->post302 = toggle;
        break;
      case 'I': /* --post303 */
        config->post303 = toggle;
        break;
      case '5': /* --noproxy */
        /* This specifies the noproxy list */
        GetStr(&config->noproxy, nextarg);
        break;
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
      case '6': /* --socks5-gssapi-service */
        GetStr(&config->socks5_gssapi_service, nextarg);
        break;
      case '7': /* --socks5-gssapi-nec*/
        config->socks5_gssapi_nec = toggle;
        break;
#endif
      case '8': /* --proxy1.0 */
        /* http 1.0 proxy */
        GetStr(&config->proxy, nextarg);
        config->proxyver = CURLPROXY_HTTP_1_0;
        break;
      case '9': /* --tftp-blksize */
        err = str2unum(&config->tftp_blksize, nextarg);
        if(err)
          return err;
        break;
      case 'A': /* --mail-from */
        GetStr(&config->mail_from, nextarg);
        break;
      case 'B': /* --mail-rcpt */
        /* append receiver to a list */
        err = add2list(&config->mail_rcpt, nextarg);
        if(err)
          return err;
        break;
      case 'C': /* --ftp-pret */
        config->ftp_pret = toggle;
        break;
      case 'D': /* --proto */
        config->proto_present = TRUE;
        if(proto2num(config, &config->proto, nextarg))
          return PARAM_BAD_USE;
        break;
      case 'E': /* --proto-redir */
        config->proto_redir_present = TRUE;
        if(proto2num(config, &config->proto_redir, nextarg))
          return PARAM_BAD_USE;
        break;
      case 'F': /* --resolve */
        err = add2list(&config->resolve, nextarg);
        if(err)
          return err;
        break;
      case 'G': /* --delegation LEVEL */
        config->gssapi_delegation = delegation(config, nextarg);
        break;
      case 'H': /* --mail-auth */
        GetStr(&config->mail_auth, nextarg);
        break;
      case 'J': /* --metalink */
        {
#ifdef USE_METALINK
          int mlmaj, mlmin, mlpatch;
          metalink_get_version(&mlmaj, &mlmin, &mlpatch);
          if((mlmaj*10000)+(mlmin*100)+mlpatch < CURL_REQ_LIBMETALINK_VERS) {
            warnf(config,
                  "--metalink option cannot be used because the version of "
                  "the linked libmetalink library is too old. "
                  "Required: %d.%d.%d, found %d.%d.%d\n",
                  CURL_REQ_LIBMETALINK_MAJOR,
                  CURL_REQ_LIBMETALINK_MINOR,
                  CURL_REQ_LIBMETALINK_PATCH,
                  mlmaj, mlmin, mlpatch);
            return PARAM_BAD_USE;
          }
          else
            config->use_metalink = toggle;
#else
          warnf(config, "--metalink option is ignored because the binary is "
                "built without the Metalink support.\n");
#endif
          break;
        }
      case 'K': /* --sasl-ir */
        config->sasl_ir = toggle;
        break;
      case 'L': /* --test-event */
#ifdef CURLDEBUG
        config->test_event_based = toggle;
#else
        warnf(config, "--test-event is ignored unless a debug build!\n");
#endif
        break;
      case 'M': /* --unix-socket */
        GetStr(&config->unix_socket_path, nextarg);
        break;
      }
      break;
    case '#': /* --progress-bar */
      if(toggle)
        global->progressmode = CURL_PROGRESS_BAR;
      else
        global->progressmode = CURL_PROGRESS_STATS;
      break;
    case ':': /* --next */
      return PARAM_NEXT_OPERATION;
    case '~': /* --xattr */
      config->xattr = toggle;
      break;
    case '0': /* --http* options */
      switch(subletter) {
      case '\0':
        /* HTTP version 1.0 */
        config->httpversion = CURL_HTTP_VERSION_1_0;
        break;
      case '1':
        /* HTTP version 1.1 */
        config->httpversion = CURL_HTTP_VERSION_1_1;
        break;
      case '2':
        /* HTTP version 2.0 */
        config->httpversion = CURL_HTTP_VERSION_2_0;
        break;
      }
      break;
    case '1': /* --tlsv1* options */
      switch(subletter) {
      case '\0':
        /* TLS version 1.x */
        config->ssl_version = CURL_SSLVERSION_TLSv1;
        break;
      case '0':
        /* TLS version 1.0 */
        config->ssl_version = CURL_SSLVERSION_TLSv1_0;
        break;
      case '1':
        /* TLS version 1.1 */
        config->ssl_version = CURL_SSLVERSION_TLSv1_1;
        break;
      case '2':
        /* TLS version 1.2 */
        config->ssl_version = CURL_SSLVERSION_TLSv1_2;
        break;
      }
      break;
    case '2':
      /* SSL version 2 */
      config->ssl_version = CURL_SSLVERSION_SSLv2;
      break;
    case '3':
      /* SSL version 3 */
      config->ssl_version = CURL_SSLVERSION_SSLv3;
      break;
    case '4':
      /* IPv4 */
      config->ip_version = 4;
      break;
    case '6':
      /* IPv6 */
      config->ip_version = 6;
      break;
    case 'a':
      /* This makes the FTP sessions use APPE instead of STOR */
      config->ftp_append = toggle;
      break;
    case 'A':
      /* This specifies the User-Agent name */
      GetStr(&config->useragent, nextarg);
      break;
    case 'b': /* cookie string coming up: */
      if(nextarg[0] == '@') {
        nextarg++;
      }
      else if(strchr(nextarg, '=')) {
        /* A cookie string must have a =-letter */
        GetStr(&config->cookie, nextarg);
        break;
      }
      /* We have a cookie file to read from! */
      GetStr(&config->cookiefile, nextarg);
      break;
    case 'B':
      /* use ASCII/text when transferring */
      config->use_ascii = toggle;
      break;
    case 'c':
      /* get the file name to dump all cookies in */
      GetStr(&config->cookiejar, nextarg);
      break;
    case 'C':
      /* This makes us continue an ftp transfer at given position */
      if(!curlx_strequal(nextarg, "-")) {
        err = str2offset(&config->resume_from, nextarg);
        if(err)
          return err;
        config->resume_from_current = FALSE;
      }
      else {
        config->resume_from_current = TRUE;
        config->resume_from = 0;
      }
      config->use_resume=TRUE;
      break;
    case 'd':
      /* postfield data */
    {
      char *postdata = NULL;
      FILE *file;
      size_t size = 0;

      if(subletter == 'e') { /* --data-urlencode*/
        /* [name]=[content], we encode the content part only
         * [name]@[file name]
         *
         * Case 2: we first load the file using that name and then encode
         * the content.
         */
        const char *p = strchr(nextarg, '=');
        size_t nlen;
        char is_file;
        if(!p)
          /* there was no '=' letter, check for a '@' instead */
          p = strchr(nextarg, '@');
        if(p) {
          nlen = p - nextarg; /* length of the name part */
          is_file = *p++; /* pass the separator */
        }
        else {
          /* neither @ nor =, so no name and it isn't a file */
          nlen = is_file = 0;
          p = nextarg;
        }
        if('@' == is_file) {
          /* a '@' letter, it means that a file name or - (stdin) follows */

          if(curlx_strequal("-", p)) {
            file = stdin;
            set_binmode(stdin);
          }
          else {
            file = fopen(p, "rb");
            if(!file)
              warnf(config,
                    "Couldn't read data from file \"%s\", this makes "
                    "an empty POST.\n", nextarg);
          }

          err = file2memory(&postdata, &size, file);

          if(file && (file != stdin))
            fclose(file);
          if(err)
            return err;
        }
        else {
          GetStr(&postdata, p);
          if(postdata)
            size = strlen(postdata);
        }

        if(!postdata) {
          /* no data from the file, point to a zero byte string to make this
             get sent as a POST anyway */
          postdata = strdup("");
          if(!postdata)
            return PARAM_NO_MEM;
          size = 0;
        }
        else {
          char *enc = curl_easy_escape(config->easy, postdata, (int)size);
          Curl_safefree(postdata); /* no matter if it worked or not */
          if(enc) {
            /* now make a string with the name from above and append the
               encoded string */
            size_t outlen = nlen + strlen(enc) + 2;
            char *n = malloc(outlen);
            if(!n) {
              curl_free(enc);
              return PARAM_NO_MEM;
            }
            if(nlen > 0) { /* only append '=' if we have a name */
              snprintf(n, outlen, "%.*s=%s", nlen, nextarg, enc);
              size = outlen-1;
            }
            else {
              strcpy(n, enc);
              size = outlen-2; /* since no '=' was inserted */
            }
            curl_free(enc);
            postdata = n;
          }
          else
            return PARAM_NO_MEM;
        }
      }
      else if('@' == *nextarg) {
        /* the data begins with a '@' letter, it means that a file name
           or - (stdin) follows */
        nextarg++; /* pass the @ */

        if(curlx_strequal("-", nextarg)) {
          file = stdin;
          if(subletter == 'b') /* forced data-binary */
            set_binmode(stdin);
        }
        else {
          file = fopen(nextarg, "rb");
          if(!file)
            warnf(config, "Couldn't read data from file \"%s\", this makes "
                  "an empty POST.\n", nextarg);
        }

        if(subletter == 'b')
          /* forced binary */
          err = file2memory(&postdata, &size, file);
        else {
          err = file2string(&postdata, file);
          if(postdata)
            size = strlen(postdata);
        }

        if(file && (file != stdin))
          fclose(file);
        if(err)
          return err;

        if(!postdata) {
          /* no data from the file, point to a zero byte string to make this
             get sent as a POST anyway */
          postdata = strdup("");
          if(!postdata)
            return PARAM_NO_MEM;
        }
      }
      else {
        GetStr(&postdata, nextarg);
        if(postdata)
          size = strlen(postdata);
      }

#ifdef CURL_DOES_CONVERSIONS
      if(subletter != 'b') {
        /* NOT forced binary, convert to ASCII */
        if(convert_to_network(postdata, strlen(postdata))) {
          Curl_safefree(postdata);
          return PARAM_NO_MEM;
        }
      }
#endif

      if(config->postfields) {
        /* we already have a string, we append this one with a separating
           &-letter */
        char *oldpost = config->postfields;
        curl_off_t oldlen = config->postfieldsize;
        curl_off_t newlen = oldlen + curlx_uztoso(size) + 2;
        config->postfields = malloc((size_t)newlen);
        if(!config->postfields) {
          Curl_safefree(oldpost);
          Curl_safefree(postdata);
          return PARAM_NO_MEM;
        }
        memcpy(config->postfields, oldpost, (size_t)oldlen);
        /* use byte value 0x26 for '&' to accommodate non-ASCII platforms */
        config->postfields[oldlen] = '\x26';
        memcpy(&config->postfields[oldlen+1], postdata, size);
        config->postfields[oldlen+1+size] = '\0';
        Curl_safefree(oldpost);
        Curl_safefree(postdata);
        config->postfieldsize += size+1;
      }
      else {
        config->postfields = postdata;
        config->postfieldsize = curlx_uztoso(size);
      }
    }
    /*
      We can't set the request type here, as this data might be used in
      a simple GET if -G is used. Already or soon.

      if(SetHTTPrequest(HTTPREQ_SIMPLEPOST, &config->httpreq)) {
        Curl_safefree(postdata);
        return PARAM_BAD_USE;
      }
    */
    break;
    case 'D':
      /* dump-header to given file name */
      GetStr(&config->headerfile, nextarg);
      break;
    case 'e':
    {
      char *ptr = strstr(nextarg, ";auto");
      if(ptr) {
        /* Automatic referer requested, this may be combined with a
           set initial one */
        config->autoreferer = TRUE;
        *ptr = 0; /* zero terminate here */
      }
      else
        config->autoreferer = FALSE;
      GetStr(&config->referer, nextarg);
    }
    break;
    case 'E':
      switch(subletter) {
      case 'a': /* CA info PEM file */
        /* CA info PEM file */
        GetStr(&config->cacert, nextarg);
        break;
      case 'b': /* cert file type */
        GetStr(&config->cert_type, nextarg);
        break;
      case 'c': /* private key file */
        GetStr(&config->key, nextarg);
        break;
      case 'd': /* private key file type */
        GetStr(&config->key_type, nextarg);
        break;
      case 'e': /* private key passphrase */
        GetStr(&config->key_passwd, nextarg);
        cleanarg(nextarg);
        break;
      case 'f': /* crypto engine */
        GetStr(&config->engine, nextarg);
        if(config->engine && curlx_raw_equal(config->engine,"list"))
          return PARAM_ENGINES_REQUESTED;
        break;
      case 'g': /* CA info PEM file */
        /* CA cert directory */
        GetStr(&config->capath, nextarg);
        break;
      case 'h': /* --pubkey public key file */
        GetStr(&config->pubkey, nextarg);
        break;
      case 'i': /* --hostpubmd5 md5 of the host public key */
        GetStr(&config->hostpubmd5, nextarg);
        if(!config->hostpubmd5 || strlen(config->hostpubmd5) != 32)
          return PARAM_BAD_USE;
        break;
      case 'j': /* CRL info PEM file */
        /* CRL file */
        GetStr(&config->crlfile, nextarg);
        break;
      case 'k': /* TLS username */
        if(curlinfo->features & CURL_VERSION_TLSAUTH_SRP)
          GetStr(&config->tls_username, nextarg);
        else
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        break;
      case 'l': /* TLS password */
        if(curlinfo->features & CURL_VERSION_TLSAUTH_SRP)
          GetStr(&config->tls_password, nextarg);
        else
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        break;
      case 'm': /* TLS authentication type */
        if(curlinfo->features & CURL_VERSION_TLSAUTH_SRP) {
          GetStr(&config->tls_authtype, nextarg);
          if(!strequal(config->tls_authtype, "SRP"))
            return PARAM_LIBCURL_DOESNT_SUPPORT; /* only support TLS-SRP */
        }
        else
          return PARAM_LIBCURL_DOESNT_SUPPORT;
        break;
      case 'n': /* no empty SSL fragments, --ssl-allow-beast */
        if(curlinfo->features & CURL_VERSION_SSL)
          config->ssl_allow_beast = toggle;
        break;

      case 'o': /* --login-options */
        GetStr(&config->login_options, nextarg);
        break;

      case 'p': /* Pinned public key DER file */
        /* Pinned public key DER file */
        GetStr(&config->pinnedpubkey, nextarg);
        break;

      case 'q': /* --cert-status */
        config->verifystatus = TRUE;
        break;

      default: /* certificate file */
      {
        char *certname, *passphrase;
        parse_cert_parameter(nextarg, &certname, &passphrase);
        Curl_safefree(config->cert);
        config->cert = certname;
        if(passphrase) {
          Curl_safefree(config->key_passwd);
          config->key_passwd = passphrase;
        }
        cleanarg(nextarg);
      }
      }
      break;
    case 'f':
      /* fail hard on errors  */
      config->failonerror = toggle;
      break;
    case 'F':
      /* "form data" simulation, this is a little advanced so lets do our best
         to sort this out slowly and carefully */
      if(formparse(config,
                   nextarg,
                   &config->httppost,
                   &config->last_post,
                   (subletter=='s')?TRUE:FALSE)) /* 's' means literal string */
        return PARAM_BAD_USE;
      if(SetHTTPrequest(config, HTTPREQ_POST, &config->httpreq))
        return PARAM_BAD_USE;
      break;

    case 'g': /* g disables URLglobbing */
      config->globoff = toggle;
      break;

    case 'G': /* HTTP GET */
      config->use_httpget = TRUE;
      break;

    case 'h': /* h for help */
      if(toggle) {
        return PARAM_HELP_REQUESTED;
      }
      /* we now actually support --no-help too! */
      break;
    case 'H':
      /* A custom header to append to a list */
      if(subletter == 'p') /* --proxy-header */
        err = add2list(&config->proxyheaders, nextarg);
      else
        err = add2list(&config->headers, nextarg);
      if(err)
        return err;
      break;
    case 'i':
      config->include_headers = toggle; /* include the headers as well in the
                                           general output stream */
      break;
    case 'j':
      config->cookiesession = toggle;
      break;
    case 'I':
      /*
       * no_body will imply include_headers later on
       */
      config->no_body = toggle;
      if(SetHTTPrequest(config,
                        (config->no_body)?HTTPREQ_HEAD:HTTPREQ_GET,
                        &config->httpreq))
        return PARAM_BAD_USE;
      break;
    case 'J': /* --remote-header-name */
      if(config->include_headers) {
        warnf(config,
              "--include and --remote-header-name cannot be combined.\n");
        return PARAM_BAD_USE;
      }
      config->content_disposition = toggle;
      break;
    case 'k': /* allow insecure SSL connects */
      config->insecure_ok = toggle;
      break;
    case 'K': /* parse config file */
      if(parseconfig(nextarg, global))
        warnf(config, "error trying read config from the '%s' file\n",
              nextarg);
      break;
    case 'l':
      config->dirlistonly = toggle; /* only list the names of the FTP dir */
      break;
    case 'L':
      config->followlocation = toggle; /* Follow Location: HTTP headers */
      switch (subletter) {
      case 't':
        /* Continue to send authentication (user+password) when following
         * locations, even when hostname changed */
        config->unrestricted_auth = toggle;
        break;
      }
      break;
    case 'm':
      /* specified max time */
      err = str2udouble(&config->timeout, nextarg);
      if(err)
        return err;
      break;
    case 'M': /* M for manual, huge help */
      if(toggle) { /* --no-manual shows no manual... */
#ifdef USE_MANUAL
        return PARAM_MANUAL_REQUESTED;
#else
        warnf(config,
              "built-in manual was disabled at build-time!\n");
        return PARAM_OPTION_UNKNOWN;
#endif
      }
      break;
    case 'n':
      switch(subletter) {
      case 'o': /* CA info PEM file */
        /* use .netrc or URL */
        config->netrc_opt = toggle;
        break;
      case 'e': /* netrc-file */
        GetStr(&config->netrc_file, nextarg);
        break;
      default:
        /* pick info from .netrc, if this is used for http, curl will
           automatically enfore user+password with the request */
        config->netrc = toggle;
        break;
      }
      break;
    case 'N':
      /* disable the output I/O buffering. note that the option is called
         --buffer but is mostly used in the negative form: --no-buffer */
      if(longopt)
        config->nobuffer = (!toggle)?TRUE:FALSE;
      else
        config->nobuffer = toggle;
      break;
    case 'O': /* --remote-name */
      if(subletter == 'a') { /* --remote-name-all */
        config->default_node_flags = toggle?GETOUT_USEREMOTE:0;
        break;
      }
      /* fall-through! */
    case 'o': /* --output */
      /* output file */
    {
      struct getout *url;
      if(config->url_out || ((config->url_out = config->url_list) != NULL)) {
        /* there's a node here, if it already is filled-in continue to find
           an "empty" node */
        while(config->url_out && (config->url_out->flags & GETOUT_OUTFILE))
          config->url_out = config->url_out->next;
      }

      /* now there might or might not be an available node to fill in! */

      if(config->url_out)
        /* existing node */
        url = config->url_out;
      else
        /* there was no free node, create one! */
        url = new_getout(config);

      if(!url)
        return PARAM_NO_MEM;
      else {
        /* fill in the outfile */
        if('o' == letter) {
          GetStr(&url->outfile, nextarg);
          url->flags &= ~GETOUT_USEREMOTE; /* switch off */
        }
        else {
          url->outfile = NULL; /* leave it */
          if(toggle)
            url->flags |= GETOUT_USEREMOTE;  /* switch on */
          else
            url->flags &= ~GETOUT_USEREMOTE; /* switch off */
        }
        url->flags |= GETOUT_OUTFILE;
      }
    }
    break;
    case 'P':
      /* This makes the FTP sessions use PORT instead of PASV */
      /* use <eth0> or <192.168.10.10> style addresses. Anything except
         this will make us try to get the "default" address.
         NOTE: this is a changed behaviour since the released 4.1!
      */
      GetStr(&config->ftpport, nextarg);
      break;
    case 'p':
      /* proxy tunnel for non-http protocols */
      config->proxytunnel = toggle;
      break;

    case 'q': /* if used first, already taken care of, we do it like
                 this so we don't cause an error! */
      break;
    case 'Q':
      /* QUOTE command to send to FTP server */
      switch(nextarg[0]) {
      case '-':
        /* prefixed with a dash makes it a POST TRANSFER one */
        nextarg++;
        err = add2list(&config->postquote, nextarg);
        break;
      case '+':
        /* prefixed with a plus makes it a just-before-transfer one */
        nextarg++;
        err = add2list(&config->prequote, nextarg);
        break;
      default:
        err = add2list(&config->quote, nextarg);
        break;
      }
      if(err)
        return err;
      break;
    case 'r':
      /* Specifying a range WITHOUT A DASH will create an illegal HTTP range
         (and won't actually be range by definition). The man page previously
         claimed that to be a good way, why this code is added to work-around
         it. */
      if(ISDIGIT(*nextarg) && !strchr(nextarg, '-')) {
        char buffer[32];
        curl_off_t off;
        warnf(config,
              "A specified range MUST include at least one dash (-). "
              "Appending one for you!\n");
        off = curlx_strtoofft(nextarg, NULL, 10);
        snprintf(buffer, sizeof(buffer), "%" CURL_FORMAT_CURL_OFF_T "-", off);
        Curl_safefree(config->range);
        config->range = strdup(buffer);
        if(!config->range)
          return PARAM_NO_MEM;
      }
      {
        /* byte range requested */
        char *tmp_range;
        tmp_range = nextarg;
        while(*tmp_range != '\0') {
          if(!ISDIGIT(*tmp_range) && *tmp_range != '-' && *tmp_range != ',') {
            warnf(config,"Invalid character is found in given range. "
                  "A specified range MUST have only digits in "
                  "\'start\'-\'stop\'. The server's response to this "
                  "request is uncertain.\n");
            break;
          }
          tmp_range++;
        }
        /* byte range requested */
        GetStr(&config->range, nextarg);
      }
      break;
    case 'R':
      /* use remote file's time */
      config->remote_time = toggle;
      break;
    case 's':
      /* don't show progress meter, don't show errors : */
      if(toggle)
        global->mute = global->noprogress = TRUE;
      else
        global->mute = global->noprogress = FALSE;
      if(global->showerror < 0)
        /* if still on the default value, set showerror to the reverse of
           toggle. This is to allow -S and -s to be used in an independent
           order but still have the same effect. */
        global->showerror = (!toggle)?TRUE:FALSE; /* toggle off */
      break;
    case 'S':
      /* show errors */
      global->showerror = toggle?1:0; /* toggle on if used with -s */
      break;
    case 't':
      /* Telnet options */
      err = add2list(&config->telnet_options, nextarg);
      if(err)
        return err;
      break;
    case 'T':
      /* we are uploading */
    {
      struct getout *url;
      if(config->url_out || ((config->url_out = config->url_list) != NULL)) {
        /* there's a node here, if it already is filled-in continue to find
           an "empty" node */
        while(config->url_out && (config->url_out->flags & GETOUT_UPLOAD))
          config->url_out = config->url_out->next;
      }

      /* now there might or might not be an available node to fill in! */

      if(config->url_out)
        /* existing node */
        url = config->url_out;
      else
        /* there was no free node, create one! */
        url = new_getout(config);

      if(!url)
        return PARAM_NO_MEM;
      else {
        url->flags |= GETOUT_UPLOAD; /* mark -T used */
        if(!*nextarg)
          url->flags |= GETOUT_NOUPLOAD;
        else {
          /* "-" equals stdin, but keep the string around for now */
          GetStr(&url->infile, nextarg);
        }
      }
    }
    break;
    case 'u':
      /* user:password  */
      GetStr(&config->userpwd, nextarg);
      cleanarg(nextarg);
      break;
    case 'U':
      /* Proxy user:password  */
      GetStr(&config->proxyuserpwd, nextarg);
      cleanarg(nextarg);
      break;
    case 'v':
      if(toggle) {
        /* the '%' thing here will cause the trace get sent to stderr */
        Curl_safefree(global->trace_dump);
        global->trace_dump = strdup("%");
        if(!global->trace_dump)
          return PARAM_NO_MEM;
        if(global->tracetype && (global->tracetype != TRACE_PLAIN))
          warnf(config,
                "-v, --verbose overrides an earlier trace/verbose option\n");
        global->tracetype = TRACE_PLAIN;
      }
      else
        /* verbose is disabled here */
        global->tracetype = TRACE_NONE;
      break;
    case 'V':
      if(toggle)    /* --no-version yields no output! */
        return PARAM_VERSION_INFO_REQUESTED;
      break;

    case 'w':
      /* get the output string */
      if('@' == *nextarg) {
        /* the data begins with a '@' letter, it means that a file name
           or - (stdin) follows */
        FILE *file;
        const char *fname;
        nextarg++; /* pass the @ */
        if(curlx_strequal("-", nextarg)) {
          fname = "<stdin>";
          file = stdin;
        }
        else {
          fname = nextarg;
          file = fopen(nextarg, "r");
        }
        err = file2string(&config->writeout, file);
        if(file && (file != stdin))
          fclose(file);
        if(err)
          return err;
        if(!config->writeout)
          warnf(config, "Failed to read %s", fname);
      }
      else
        GetStr(&config->writeout, nextarg);
      break;
    case 'x':
      /* proxy */
      GetStr(&config->proxy, nextarg);
      config->proxyver = CURLPROXY_HTTP;
      break;
    case 'X':
      /* set custom request */
      GetStr(&config->customrequest, nextarg);
      break;
    case 'y':
      /* low speed time */
      err = str2unum(&config->low_speed_time, nextarg);
      if(err)
        return err;
      if(!config->low_speed_limit)
        config->low_speed_limit = 1;
      break;
    case 'Y':
      /* low speed limit */
      err = str2unum(&config->low_speed_limit, nextarg);
      if(err)
        return err;
      if(!config->low_speed_time)
        config->low_speed_time = 30;
      break;
    case 'z': /* time condition coming up */
      switch(*nextarg) {
      case '+':
        nextarg++;
        /* FALLTHROUGH */
      default:
        /* If-Modified-Since: (section 14.28 in RFC2068) */
        config->timecond = CURL_TIMECOND_IFMODSINCE;
        break;
      case '-':
        /* If-Unmodified-Since:  (section 14.24 in RFC2068) */
        config->timecond = CURL_TIMECOND_IFUNMODSINCE;
        nextarg++;
        break;
      case '=':
        /* Last-Modified:  (section 14.29 in RFC2068) */
        config->timecond = CURL_TIMECOND_LASTMOD;
        nextarg++;
        break;
      }
      now = time(NULL);
      config->condtime=curl_getdate(nextarg, &now);
      if(-1 == (int)config->condtime) {
        /* now let's see if it is a file name to get the time from instead! */
        struct_stat statbuf;
        if(-1 == stat(nextarg, &statbuf)) {
          /* failed, remove time condition */
          config->timecond = CURL_TIMECOND_NONE;
          warnf(config,
                "Illegal date format for -z, --timecond (and not "
                "a file name). Disabling time condition. "
                "See curl_getdate(3) for valid date syntax.\n");
        }
        else {
          /* pull the time out from the file */
          config->condtime = statbuf.st_mtime;
        }
      }
      break;
    default: /* unknown flag */
      return PARAM_OPTION_UNKNOWN;
    }
    hit = -1;

  } while(!longopt && !singleopt && *++parse && !*usedarg);

  return PARAM_OK;
}