        default: {
            /* oo-er! an error */
            return rv;
        }
        }
    }

    /* if Cache-Control: only-if-cached, and not cached, return 504 */
    if (cache->control_in.only_if_cached) {
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, r, APLOGNO(00696)
                "cache: 'only-if-cached' requested and no cached entity, "
                "returning 504 Gateway Timeout for: %s", r->uri);
        return HTTP_GATEWAY_TIME_OUT;
    }

    return DECLINED;
}

apr_status_t cache_generate_key_default(request_rec *r, apr_pool_t* p,
        const char **key)
{
    cache_server_conf *conf;
    char *port_str, *hn, *lcs;
    const char *hostname, *scheme;
    int i;
    char *path, *querystring;

    if (*key) {
        /*
         * We have been here before during the processing of this request.
         */
        return APR_SUCCESS;
    }

    /*
     * Get the module configuration. We need this for the CacheIgnoreQueryString
     * option below.
