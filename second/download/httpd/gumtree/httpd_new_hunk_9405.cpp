    }

    /* perform sub-request for the file name without the suffix */
    result = 0;
    sub_filename = ap_pstrndup(r->pool, r->filename, suffix_pos);
#if MIME_MAGIC_DEBUG
    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_DEBUG, r,
		MODNAME ": subrequest lookup for %s", sub_filename);
#endif /* MIME_MAGIC_DEBUG */
    sub = ap_sub_req_lookup_file(sub_filename, r);

    /* extract content type/encoding/language from sub-request */
    if (sub->content_type) {
	r->content_type = ap_pstrdup(r->pool, sub->content_type);
#if MIME_MAGIC_DEBUG
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_DEBUG, r,
		    MODNAME ": subrequest %s got %s",
		    sub_filename, r->content_type);
#endif /* MIME_MAGIC_DEBUG */
	if (sub->content_encoding)
	    r->content_encoding =
		ap_pstrdup(r->pool, sub->content_encoding);
++ apache_1.3.2/src/modules/standard/mod_negotiation.c	1998-08-07 01:31:00.000000000 +0800
