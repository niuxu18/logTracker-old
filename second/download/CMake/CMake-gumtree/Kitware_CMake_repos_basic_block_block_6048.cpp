{
  char *prox_portno;
  char *endofprot;

  /* We use 'proxyptr' to point to the proxy name from now on... */
  char *proxyptr;
  char *portptr;
  char *atsign;
  long port = -1;
  char *proxyuser = NULL;
  char *proxypasswd = NULL;
  bool sockstype;

  /* We do the proxy host string parsing here. We want the host name and the
   * port name. Accept a protocol:// prefix
   */

  /* Parse the protocol part if present */
  endofprot = strstr(proxy, "://");
  if(endofprot) {
    proxyptr = endofprot+3;
    if(checkprefix("https", proxy))
      proxytype = CURLPROXY_HTTPS;
    else if(checkprefix("socks5h", proxy))
      proxytype = CURLPROXY_SOCKS5_HOSTNAME;
    else if(checkprefix("socks5", proxy))
      proxytype = CURLPROXY_SOCKS5;
    else if(checkprefix("socks4a", proxy))
      proxytype = CURLPROXY_SOCKS4A;
    else if(checkprefix("socks4", proxy) || checkprefix("socks", proxy))
      proxytype = CURLPROXY_SOCKS4;
    else if(checkprefix("http:", proxy))
      ; /* leave it as HTTP or HTTP/1.0 */
    else {
      /* Any other xxx:// reject! */
      failf(data, "Unsupported proxy scheme for \'%s\'", proxy);
      return CURLE_COULDNT_CONNECT;
    }
  }
  else
    proxyptr = proxy; /* No xxx:// head: It's a HTTP proxy */

#ifndef HTTPS_PROXY_SUPPORT
  if(proxytype == CURLPROXY_HTTPS) {
    failf(data, "Unsupported proxy \'%s\'"
                ", libcurl is built without the HTTPS-proxy support.", proxy);
    return CURLE_NOT_BUILT_IN;
  }
#endif

  sockstype = proxytype == CURLPROXY_SOCKS5_HOSTNAME ||
              proxytype == CURLPROXY_SOCKS5 ||
              proxytype == CURLPROXY_SOCKS4A ||
              proxytype == CURLPROXY_SOCKS4;

  /* Is there a username and password given in this proxy url? */
  atsign = strchr(proxyptr, '@');
  if(atsign) {
    CURLcode result =
      parse_login_details(proxyptr, atsign - proxyptr,
                              &proxyuser, &proxypasswd, NULL);
    if(result)
      return result;
    proxyptr = atsign + 1;
  }

  /* start scanning for port number at this point */
  portptr = proxyptr;

  /* detect and extract RFC6874-style IPv6-addresses */
  if(*proxyptr == '[') {
    char *ptr = ++proxyptr; /* advance beyond the initial bracket */
    while(*ptr && (ISXDIGIT(*ptr) || (*ptr == ':') || (*ptr == '.')))
      ptr++;
    if(*ptr == '%') {
      /* There might be a zone identifier */
      if(strncmp("%25", ptr, 3))
        infof(data, "Please URL encode %% as %%25, see RFC 6874.\n");
      ptr++;
      /* Allow unreserved characters as defined in RFC 3986 */
      while(*ptr && (ISALPHA(*ptr) || ISXDIGIT(*ptr) || (*ptr == '-') ||
                     (*ptr == '.') || (*ptr == '_') || (*ptr == '~')))
        ptr++;
    }
    if(*ptr == ']')
      /* yeps, it ended nicely with a bracket as well */
      *ptr++ = 0;
    else
      infof(data, "Invalid IPv6 address format\n");
    portptr = ptr;
    /* Note that if this didn't end with a bracket, we still advanced the
     * proxyptr first, but I can't see anything wrong with that as no host
     * name nor a numeric can legally start with a bracket.
     */
  }

  /* Get port number off proxy.server.com:1080 */
  prox_portno = strchr(portptr, ':');
  if(prox_portno) {
    char *endp = NULL;

    *prox_portno = 0x0; /* cut off number from host name */
    prox_portno ++;
    /* now set the local port number */
    port = strtol(prox_portno, &endp, 10);
    if((endp && *endp && (*endp != '/') && (*endp != ' ')) ||
       (port < 0) || (port > 65535)) {
      /* meant to detect for example invalid IPv6 numerical addresses without
         brackets: "2a00:fac0:a000::7:13". Accept a trailing slash only
         because we then allow "URL style" with the number followed by a
         slash, used in curl test cases already. Space is also an acceptable
         terminating symbol. */
      infof(data, "No valid port number in proxy string (%s)\n",
            prox_portno);
    }
    else
      conn->port = port;
  }
  else {
    if(proxyptr[0]=='/')
      /* If the first character in the proxy string is a slash, fail
         immediately. The following code will otherwise clear the string which
         will lead to code running as if no proxy was set! */
      return CURLE_COULDNT_RESOLVE_PROXY;

    /* without a port number after the host name, some people seem to use
       a slash so we strip everything from the first slash */
    atsign = strchr(proxyptr, '/');
    if(atsign)
      *atsign = '\0'; /* cut off path part from host name */

    if(data->set.proxyport)
      /* None given in the proxy string, then get the default one if it is
         given */
      port = data->set.proxyport;
    else {
      if(proxytype == CURLPROXY_HTTPS)
        port = CURL_DEFAULT_HTTPS_PROXY_PORT;
      else
        port = CURL_DEFAULT_PROXY_PORT;
    }
  }

  if(*proxyptr) {
    struct proxy_info *proxyinfo =
      sockstype ? &conn->socks_proxy : &conn->http_proxy;
    proxyinfo->proxytype = proxytype;

    if(proxyuser) {
      /* found user and password, rip them out.  note that we are unescaping
         them, as there is otherwise no way to have a username or password
         with reserved characters like ':' in them. */
      Curl_safefree(proxyinfo->user);
      proxyinfo->user = curl_easy_unescape(data, proxyuser, 0, NULL);
      Curl_safefree(proxyuser);

      if(!proxyinfo->user) {
        Curl_safefree(proxypasswd);
        return CURLE_OUT_OF_MEMORY;
      }

      Curl_safefree(proxyinfo->passwd);
      if(proxypasswd && strlen(proxypasswd) < MAX_CURL_PASSWORD_LENGTH)
        proxyinfo->passwd = curl_easy_unescape(data, proxypasswd, 0, NULL);
      else
        proxyinfo->passwd = strdup("");
      Curl_safefree(proxypasswd);

      if(!proxyinfo->passwd)
        return CURLE_OUT_OF_MEMORY;

      conn->bits.proxy_user_passwd = TRUE; /* enable it */
    }

    if(port >= 0) {
      proxyinfo->port = port;
      if(conn->port < 0 || sockstype || !conn->socks_proxy.host.rawalloc)
        conn->port = port;
    }

    /* now, clone the cleaned proxy host name */
    Curl_safefree(proxyinfo->host.rawalloc);
    proxyinfo->host.rawalloc = strdup(proxyptr);
    proxyinfo->host.name = proxyinfo->host.rawalloc;

    if(!proxyinfo->host.rawalloc)
      return CURLE_OUT_OF_MEMORY;
  }

  Curl_safefree(proxyuser);
  Curl_safefree(proxypasswd);

  return CURLE_OK;
}