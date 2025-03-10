static CURLcode parseurlandfillconn(struct Curl_easy *data,
                                    struct connectdata *conn,
                                    bool *prot_missing,
                                    char **userp, char **passwdp,
                                    char **optionsp)
{
  char *at;
  char *fragment;
  char *path = data->state.path;
  char *query;
  int i;
  int rc;
  const char *protop = "";
  CURLcode result;
  bool rebuild_url = FALSE;
  bool url_has_scheme = FALSE;
  char protobuf[16];

  *prot_missing = FALSE;

  /* We might pass the entire URL into the request so we need to make sure
   * there are no bad characters in there.*/
  if(strpbrk(data->change.url, "\r\n")) {
    failf(data, "Illegal characters found in URL");
    return CURLE_URL_MALFORMAT;
  }

  /*************************************************************
   * Parse the URL.
   *
   * We need to parse the url even when using the proxy, because we will need
   * the hostname and port in case we are trying to SSL connect through the
   * proxy -- and we don't know if we will need to use SSL until we parse the
   * url ...
   ************************************************************/
  if(data->change.url[0] == ':') {
    failf(data, "Bad URL, colon is first character");
    return CURLE_URL_MALFORMAT;
  }

  /* Make sure we don't mistake a drive letter for a scheme, for example:
     curld --proto-default file c:/foo/bar.txt */
  if((('a' <= data->change.url[0] && data->change.url[0] <= 'z') ||
      ('A' <= data->change.url[0] && data->change.url[0] <= 'Z')) &&
     data->change.url[1] == ':' && data->set.str[STRING_DEFAULT_PROTOCOL] &&
     strcasecompare(data->set.str[STRING_DEFAULT_PROTOCOL], "file")) {
    ; /* do nothing */
  }
  else { /* check for a scheme */
    for(i = 0; i < 16 && data->change.url[i]; ++i) {
      if(data->change.url[i] == '/')
        break;
      if(data->change.url[i] == ':') {
        url_has_scheme = TRUE;
        break;
      }
    }
  }

  /* handle the file: scheme */
  if((url_has_scheme && strncasecompare(data->change.url, "file:", 5)) ||
     (!url_has_scheme && data->set.str[STRING_DEFAULT_PROTOCOL] &&
      strcasecompare(data->set.str[STRING_DEFAULT_PROTOCOL], "file"))) {
    bool path_has_drive = FALSE;

    if(url_has_scheme)
      rc = sscanf(data->change.url, "%*15[^\n/:]:%[^\n]", path);
    else
      rc = sscanf(data->change.url, "%[^\n]", path);

    if(rc != 1) {
      failf(data, "Bad URL");
      return CURLE_URL_MALFORMAT;
    }

    if(url_has_scheme && path[0] == '/' && path[1] == '/') {
      /* Allow omitted hostname (e.g. file:/<path>).  This is not strictly
       * speaking a valid file: URL by RFC 1738, but treating file:/<path> as
       * file://localhost/<path> is similar to how other schemes treat missing
       * hostnames.  See RFC 1808. */

      /* This cannot be done with strcpy() in a portable manner, since the
         memory areas overlap! */
      memmove(path, path + 2, strlen(path + 2)+1);
    }

    /* the path may start with a drive letter. for backwards compatibility
       we skip some processing on those paths. */
    path_has_drive = (('a' <= path[0] && path[0] <= 'z') ||
                      ('A' <= path[0] && path[0] <= 'Z')) && path[1] == ':';

    /*
     * we deal with file://<host>/<path> differently since it supports no
     * hostname other than "localhost" and "127.0.0.1", which is unique among
     * the URL protocols specified in RFC 1738
     */
    if(path[0] != '/' && !path_has_drive) {
      /* the URL includes a host name, it must match "localhost" or
         "127.0.0.1" to be valid */
      char *ptr;
      if(!checkprefix("localhost/", path) &&
         !checkprefix("127.0.0.1/", path)) {
        failf(data, "Invalid file://hostname/, "
                    "expected localhost or 127.0.0.1 or none");
        return CURLE_URL_MALFORMAT;
      }
      ptr = &path[9]; /* now points to the slash after the host */

      /* there was a host name and slash present

         RFC1738 (section 3.1, page 5) says:

         The rest of the locator consists of data specific to the scheme,
         and is known as the "url-path". It supplies the details of how the
         specified resource can be accessed. Note that the "/" between the
         host (or port) and the url-path is NOT part of the url-path.

         As most agents use file://localhost/foo to get '/foo' although the
         slash preceding foo is a separator and not a slash for the path,
         a URL as file://localhost//foo must be valid as well, to refer to
         the same file with an absolute path.
      */

      if('/' == ptr[1])
        /* if there was two slashes, we skip the first one as that is then
           used truly as a separator */
        ptr++;

      /* This cannot be made with strcpy, as the memory chunks overlap! */
      memmove(path, ptr, strlen(ptr)+1);

      path_has_drive = (('a' <= path[0] && path[0] <= 'z') ||
                        ('A' <= path[0] && path[0] <= 'Z')) && path[1] == ':';
    }

#if !defined(MSDOS) && !defined(WIN32) && !defined(__CYGWIN__)
    if(path_has_drive) {
      failf(data, "File drive letters are only accepted in MSDOS/Windows.");
      return CURLE_URL_MALFORMAT;
    }
#endif

    protop = "file"; /* protocol string */
    *prot_missing = !url_has_scheme;
  }
  else {
    /* clear path */
    char slashbuf[4];
    path[0]=0;

    rc = sscanf(data->change.url,
                "%15[^\n/:]:%3[/]%[^\n/?#]%[^\n]",
                protobuf, slashbuf, conn->host.name, path);
    if(2 == rc) {
      failf(data, "Bad URL");
      return CURLE_URL_MALFORMAT;
    }
    if(3 > rc) {

      /*
       * The URL was badly formatted, let's try the browser-style _without_
       * protocol specified like 'http://'.
       */
      rc = sscanf(data->change.url, "%[^\n/?#]%[^\n]", conn->host.name, path);
      if(1 > rc) {
        /*
         * We couldn't even get this format.
         * djgpp 2.04 has a sscanf() bug where 'conn->host.name' is
         * assigned, but the return value is EOF!
         */
#if defined(__DJGPP__) && (DJGPP_MINOR == 4)
        if(!(rc == -1 && *conn->host.name))
#endif
        {
          failf(data, "<url> malformed");
          return CURLE_URL_MALFORMAT;
        }
      }

      /*
       * Since there was no protocol part specified in the URL use the
       * user-specified default protocol. If we weren't given a default make a
       * guess by matching some protocols against the host's outermost
       * sub-domain name. Finally if there was no match use HTTP.
       */

      protop = data->set.str[STRING_DEFAULT_PROTOCOL];
      if(!protop) {
        /* Note: if you add a new protocol, please update the list in
         * lib/version.c too! */
        if(checkprefix("FTP.", conn->host.name))
          protop = "ftp";
        else if(checkprefix("DICT.", conn->host.name))
          protop = "DICT";
        else if(checkprefix("LDAP.", conn->host.name))
          protop = "LDAP";
        else if(checkprefix("IMAP.", conn->host.name))
          protop = "IMAP";
        else if(checkprefix("SMTP.", conn->host.name))
          protop = "smtp";
        else if(checkprefix("POP3.", conn->host.name))
          protop = "pop3";
        else
          protop = "http";
      }

      *prot_missing = TRUE; /* not given in URL */
    }
    else {
      size_t s = strlen(slashbuf);
      protop = protobuf;
      if(s != 2) {
        infof(data, "Unwillingly accepted illegal URL using %d slash%s!\n",
              s, s>1?"es":"");

        if(data->change.url_alloc)
          free(data->change.url);
        /* repair the URL to use two slashes */
        data->change.url = aprintf("%s://%s%s",
                                   protobuf, conn->host.name, path);
        if(!data->change.url)
          return CURLE_OUT_OF_MEMORY;
        data->change.url_alloc = TRUE;
      }
    }
  }

  /* We search for '?' in the host name (but only on the right side of a
   * @-letter to allow ?-letters in username and password) to handle things
   * like http://example.com?param= (notice the missing '/').
   */
  at = strchr(conn->host.name, '@');
  if(at)
    query = strchr(at+1, '?');
  else
    query = strchr(conn->host.name, '?');

  if(query) {
    /* We must insert a slash before the '?'-letter in the URL. If the URL had
       a slash after the '?', that is where the path currently begins and the
       '?string' is still part of the host name.

       We must move the trailing part from the host name and put it first in
       the path. And have it all prefixed with a slash.
    */

    size_t hostlen = strlen(query);
    size_t pathlen = strlen(path);

    /* move the existing path plus the zero byte forward, to make room for
       the host-name part */
    memmove(path+hostlen+1, path, pathlen+1);

     /* now copy the trailing host part in front of the existing path */
    memcpy(path+1, query, hostlen);

    path[0]='/'; /* prepend the missing slash */
    rebuild_url = TRUE;

    *query=0; /* now cut off the hostname at the ? */
  }
  else if(!path[0]) {
    /* if there's no path set, use a single slash */
    strcpy(path, "/");
    rebuild_url = TRUE;
  }

  /* If the URL is malformatted (missing a '/' after hostname before path) we
   * insert a slash here. The only letters except '/' that can start a path is
   * '?' and '#' - as controlled by the two sscanf() patterns above.
   */
  if(path[0] != '/') {
    /* We need this function to deal with overlapping memory areas. We know
       that the memory area 'path' points to is 'urllen' bytes big and that
       is bigger than the path. Use +1 to move the zero byte too. */
    memmove(&path[1], path, strlen(path)+1);
    path[0] = '/';
    rebuild_url = TRUE;
  }
  else if(!data->set.path_as_is) {
    /* sanitise paths and remove ../ and ./ sequences according to RFC3986 */
    char *newp = Curl_dedotdotify(path);
    if(!newp)
      return CURLE_OUT_OF_MEMORY;

    if(strcmp(newp, path)) {
      rebuild_url = TRUE;
      free(data->state.pathbuffer);
      data->state.pathbuffer = newp;
      data->state.path = newp;
      path = newp;
    }
    else
      free(newp);
  }

  /*
   * "rebuild_url" means that one or more URL components have been modified so
   * we need to generate an updated full version.  We need the corrected URL
   * when communicating over HTTP proxy and we don't know at this point if
   * we're using a proxy or not.
   */
  if(rebuild_url) {
    char *reurl;

    size_t plen = strlen(path); /* new path, should be 1 byte longer than
                                   the original */
    size_t prefixlen = strlen(conn->host.name);

    if(!*prot_missing) {
      size_t protolen = strlen(protop);

      if(curl_strnequal(protop, data->change.url, protolen))
        prefixlen += protolen;
      else {
        failf(data, "<url> malformed");
        return CURLE_URL_MALFORMAT;
      }

      if(curl_strnequal("://", &data->change.url[protolen], 3))
        prefixlen += 3;
      /* only file: is allowed to omit one or both slashes */
      else if(curl_strnequal("file:", data->change.url, 5))
        prefixlen += 1 + (data->change.url[5] == '/');
      else {
        failf(data, "<url> malformed");
        return CURLE_URL_MALFORMAT;
      }
    }

    reurl = malloc(prefixlen + plen + 1);
    if(!reurl)
      return CURLE_OUT_OF_MEMORY;

    /* copy the prefix */
    memcpy(reurl, data->change.url, prefixlen);

    /* append the trailing piece + zerobyte */
    memcpy(&reurl[prefixlen], path, plen + 1);

    /* possible free the old one */
    if(data->change.url_alloc) {
      Curl_safefree(data->change.url);
      data->change.url_alloc = FALSE;
    }

    infof(data, "Rebuilt URL to: %s\n", reurl);

    data->change.url = reurl;
    data->change.url_alloc = TRUE; /* free this later */
  }

  result = findprotocol(data, conn, protop);
  if(result)
    return result;

  /*
   * Parse the login details from the URL and strip them out of
   * the host name
   */
  result = parse_url_login(data, conn, userp, passwdp, optionsp);
  if(result)
    return result;

  if(conn->host.name[0] == '[') {
    /* This looks like an IPv6 address literal.  See if there is an address
       scope if there is no location header */
    char *percent = strchr(conn->host.name, '%');
    if(percent) {
      unsigned int identifier_offset = 3;
      char *endp;
      unsigned long scope;
      if(strncmp("%25", percent, 3) != 0) {
        infof(data,
              "Please URL encode %% as %%25, see RFC 6874.\n");
        identifier_offset = 1;
      }
      scope = strtoul(percent + identifier_offset, &endp, 10);
      if(*endp == ']') {
        /* The address scope was well formed.  Knock it out of the
           hostname. */
        memmove(percent, endp, strlen(endp)+1);
        conn->scope_id = (unsigned int)scope;
      }
      else {
        /* Zone identifier is not numeric */
#if defined(HAVE_NET_IF_H) && defined(IFNAMSIZ) && defined(HAVE_IF_NAMETOINDEX)
        char ifname[IFNAMSIZ + 2];
        char *square_bracket;
        unsigned int scopeidx = 0;
        strncpy(ifname, percent + identifier_offset, IFNAMSIZ + 2);
        /* Ensure nullbyte termination */
        ifname[IFNAMSIZ + 1] = '\0';
        square_bracket = strchr(ifname, ']');
        if(square_bracket) {
          /* Remove ']' */
          *square_bracket = '\0';
          scopeidx = if_nametoindex(ifname);
          if(scopeidx == 0) {
            infof(data, "Invalid network interface: %s; %s\n", ifname,
                  strerror(errno));
          }
        }
        if(scopeidx > 0) {
          char *p = percent + identifier_offset + strlen(ifname);

          /* Remove zone identifier from hostname */
          memmove(percent, p, strlen(p) + 1);
          conn->scope_id = scopeidx;
        }
        else
#endif /* HAVE_NET_IF_H && IFNAMSIZ */
          infof(data, "Invalid IPv6 address format\n");
      }
    }
  }

  if(data->set.scope_id)
    /* Override any scope that was set above.  */
    conn->scope_id = data->set.scope_id;

  /* Remove the fragment part of the path. Per RFC 2396, this is always the
     last part of the URI. We are looking for the first '#' so that we deal
     gracefully with non conformant URI such as http://example.com#foo#bar. */
  fragment = strchr(path, '#');
  if(fragment) {
    *fragment = 0;

    /* we know the path part ended with a fragment, so we know the full URL
       string does too and we need to cut it off from there so it isn't used
       over proxy */
    fragment = strchr(data->change.url, '#');
    if(fragment)
      *fragment = 0;
  }

  /*
   * So if the URL was A://B/C#D,
   *   protop is A
   *   conn->host.name is B
   *   data->state.path is /C
   */
  return CURLE_OK;
}