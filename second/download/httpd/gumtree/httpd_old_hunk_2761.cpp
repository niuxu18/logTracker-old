    kbcount = 0;
    short_report = 0;
    no_table_report = 0;

    pid_buffer = apr_palloc(r->pool, server_limit * sizeof(pid_t));
    stat_buffer = apr_palloc(r->pool, server_limit * thread_limit * sizeof(char));

    nowtime = apr_time_now();
    tu = ts = tcu = tcs = 0;

    if (!ap_exists_scoreboard_image()) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                      "Server status unavailable in inetd mode");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    r->allowed = (AP_METHOD_BIT << M_GET);
    if (r->method_number != M_GET)
