static int parse_url(char *url)
{
    char *cp;
    char *h;
    char *scope_id;
    apr_status_t rv;

    /* Save a copy for the proxy */
    fullurl = apr_pstrdup(cntxt, url);

    if (strlen(url) > 7 && strncmp(url, "http://", 7) == 0) {
	url += 7;
#ifdef USE_SSL
	ssl = 0;
#endif
    }
    else
#ifdef USE_SSL
    if (strlen(url) > 8 && strncmp(url, "https://", 8) == 0) {
	url += 8;
	ssl = 1;
    }
#else
    if (strlen(url) > 8 && strncmp(url, "https://", 8) == 0) {
	fprintf(stderr, "SSL not compiled in; no https support\n");
	exit(1);
    }
#endif

    if ((cp = strchr(url, '/')) == NULL)
	return 1;
    h = apr_palloc(cntxt, cp - url + 1);
    memcpy(h, url, cp - url);
    h[cp - url] = '\0';
    rv = apr_parse_addr_port(&hostname, &scope_id, &port, h, cntxt);
    if (rv != APR_SUCCESS || !hostname || scope_id) {
	return 1;
    }
    path = apr_pstrdup(cntxt, cp);
    *cp = '\0';
    if (*url == '[') {		/* IPv6 numeric address string */
	host_field = apr_psprintf(cntxt, "[%s]", hostname);
    }
    else {
	host_field = hostname;
    }

    if (port == 0) {		/* no port specified */
#ifdef USE_SSL
        if (ssl == 1)
            port = 443;
        else
#endif
            port = 80;
    }

    if ((
#ifdef USE_SSL
         (ssl == 1) && (port != 443)) || (( ssl == 0 ) && 
#endif
         (port != 80)))
    {
	colonhost = apr_psprintf(cntxt,":%d",port);
    } else
	colonhost = "";
    return 0;
}