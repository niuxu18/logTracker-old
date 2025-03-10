    }
    r->hostname = host;
    return;

bad:
    r->status = HTTP_BAD_REQUEST;
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(00550)
                  "Client sent malformed Host header: %s",
                  r->hostname);
    return;
}


/* return 1 if host matches ServerName or ServerAliases */
static int matches_aliases(server_rec *s, const char *host)
