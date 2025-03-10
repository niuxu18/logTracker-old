static char *detect_proxy(struct connectdata *conn)
{
  char *proxy = NULL;

#ifndef CURL_DISABLE_HTTP
  /* If proxy was not specified, we check for default proxy environment
   * variables, to enable i.e Lynx compliance:
   *
   * http_proxy=http://some.server.dom:port/
   * https_proxy=http://some.server.dom:port/
   * ftp_proxy=http://some.server.dom:port/
   * no_proxy=domain1.dom,host.domain2.dom
   *   (a comma-separated list of hosts which should
   *   not be proxied, or an asterisk to override
   *   all proxy variables)
   * all_proxy=http://some.server.dom:port/
   *   (seems to exist for the CERN www lib. Probably
   *   the first to check for.)
   *
   * For compatibility, the all-uppercase versions of these variables are
   * checked if the lowercase versions don't exist.
   */
  char *no_proxy=NULL;
  char proxy_env[128];

  no_proxy=curl_getenv("no_proxy");
  if(!no_proxy)
    no_proxy=curl_getenv("NO_PROXY");

  if(!check_noproxy(conn->host.name, no_proxy)) {
    /* It was not listed as without proxy */
    const char *protop = conn->handler->scheme;
    char *envp = proxy_env;
    char *prox;

    /* Now, build <protocol>_proxy and check for such a one to use */
    while(*protop)
      *envp++ = (char)tolower((int)*protop++);

    /* append _proxy */
    strcpy(envp, "_proxy");

    /* read the protocol proxy: */
    prox=curl_getenv(proxy_env);

    /*
     * We don't try the uppercase version of HTTP_PROXY because of
     * security reasons:
     *
     * When curl is used in a webserver application
     * environment (cgi or php), this environment variable can
     * be controlled by the web server user by setting the
     * http header 'Proxy:' to some value.
     *
     * This can cause 'internal' http/ftp requests to be
     * arbitrarily redirected by any external attacker.
     */
    if(!prox && !Curl_raw_equal("http_proxy", proxy_env)) {
      /* There was no lowercase variable, try the uppercase version: */
      Curl_strntoupper(proxy_env, proxy_env, sizeof(proxy_env));
      prox=curl_getenv(proxy_env);
    }

    if(prox && *prox) { /* don't count "" strings */
      proxy = prox; /* use this */
    }
    else {
      proxy = curl_getenv("all_proxy"); /* default proxy to use */
      if(!proxy)
        proxy=curl_getenv("ALL_PROXY");
    }
  } /* if(!check_noproxy(conn->host.name, no_proxy)) - it wasn't specified
       non-proxy */
  if(no_proxy)
    free(no_proxy);

#else /* !CURL_DISABLE_HTTP */

  (void)conn;
#endif /* CURL_DISABLE_HTTP */

  return proxy;
}