int cache_select_url(request_rec *r, const char *types, char *url)
{
    const char *next = types;
    const char *type;
    apr_status_t rv;
    cache_handle_t *h;
    char *key;
    cache_request_rec *cache = (cache_request_rec *) 
                         ap_get_module_config(r->request_config, &cache_module);

    rv =  cache_generate_key(r,r->pool,&key);
    if (rv != APR_SUCCESS) {
        return rv;
    }
    /* go through the cache types till we get a match */
    h = cache->handle = apr_palloc(r->pool, sizeof(cache_handle_t));

    while (next) {
        type = ap_cache_tokstr(r->pool, next, &next);
        switch ((rv = cache_run_open_entity(h, r, type, key))) {
        case OK: {
            char *vary = NULL;
            if (cache_read_entity_headers(h, r) != APR_SUCCESS) {
                /* TODO: Handle this error */
                return DECLINED;
            }

            /*
             * Check Content-Negotiation - Vary
             * 
             * At this point we need to make sure that the object we found in the cache
             * is the same object that would be delivered to the client, when the
             * effects of content negotiation are taken into effect.
             * 
             * In plain english, we want to make sure that a language-negotiated
             * document in one language is not given to a client asking for a
             * language negotiated document in a different language by mistake.
             * 
             * This code makes the assumption that the storage manager will
             * cache the req_hdrs if the response contains a Vary
             * header.
             * 
             * RFC2616 13.6 and 14.44 describe the Vary mechanism.
             */
            vary = apr_pstrdup(r->pool, apr_table_get(r->headers_out, "Vary"));
            while (vary && *vary) {
                char *name = vary;
                const char *h1, *h2;

                /* isolate header name */
                while (*vary && !apr_isspace(*vary) && (*vary != ','))
                    ++vary;
                while (*vary && (apr_isspace(*vary) || (*vary == ','))) {
                    *vary = '\0';
                    ++vary;
                }

                /*
                 * is this header in the request and the header in the cached
                 * request identical? If not, we give up and do a straight get
                 */
                h1 = apr_table_get(r->headers_in, name);
                h2 = apr_table_get(h->req_hdrs, name);
                if (h1 == h2) {
                    /* both headers NULL, so a match - do nothing */
                }
                else if (h1 && h2 && !strcmp(h1, h2)) {
                    /* both headers exist and are equal - do nothing */
                }
                else {
                    /* headers do not match, so Vary failed */
                    ap_log_error(APLOG_MARK, APLOG_INFO, APR_SUCCESS, r->server,
                                 "cache_select_url(): Vary header mismatch - Cached document cannot be used. \n");
                    apr_table_clear(r->headers_out);
                    r->status_line = NULL;
                    cache->handle = NULL;
                    return DECLINED;
                }
            }
            return OK;
        }
        case DECLINED: {
            /* try again with next cache type */
            continue;
        }
        default: {
            /* oo-er! an error */
            cache->handle = NULL;
            return rv;
        }
        }
    }
    cache->handle = NULL;
    return DECLINED;
}