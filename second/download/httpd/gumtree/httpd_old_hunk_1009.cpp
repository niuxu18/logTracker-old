	connecthost = apr_pstrdup(cntxt, hostname);
	connectport = port;
    }

    if (!use_html) {
	printf("Benchmarking %s ", hostname);
	if (isproxy)
	    printf("[through %s:%d] ", proxyhost, proxyport);
	printf("(be patient)%s",
	       (heartbeatres ? "\n" : "..."));
	fflush(stdout);
    }

    now = apr_time_now();

    con = calloc(concurrency * sizeof(struct connection), 1);
    
    stats = calloc(requests * sizeof(struct data), 1);

    if ((status = apr_pollset_create(&readbits, concurrency, cntxt, 0)) != APR_SUCCESS) {
        apr_err("apr_pollset_create failed", status);
    }

    /* setup request */
    if (posting <= 0) {
	sprintf(request, "%s %s HTTP/1.0\r\n"
		"User-Agent: ApacheBench/%s\r\n"
		"%s" "%s" "%s"
		"Host: %s%s\r\n"
		"Accept: */*\r\n"
		"%s" "\r\n",
		(posting == 0) ? "GET" : "HEAD",
		(isproxy) ? fullurl : path,
		AP_AB_BASEREVISION,
		keepalive ? "Connection: Keep-Alive\r\n" : "",
		cookie, auth, host_field, colonhost, hdrs);
    }
    else {
	sprintf(request, "POST %s HTTP/1.0\r\n"
		"User-Agent: ApacheBench/%s\r\n"
		"%s" "%s" "%s"
		"Host: %s%s\r\n"
		"Accept: */*\r\n"
		"Content-length: %" APR_SIZE_T_FMT "\r\n"
		"Content-type: %s\r\n"
		"%s"
		"\r\n",
		(isproxy) ? fullurl : path,
		AP_AB_BASEREVISION,
		keepalive ? "Connection: Keep-Alive\r\n" : "",
		cookie, auth,
		host_field, colonhost, postlen,
		(content_type[0]) ? content_type : "text/plain", hdrs);
    }

    if (verbosity >= 2)
	printf("INFO: POST header == \n---\n%s\n---\n", request);

    reqlen = strlen(request);
