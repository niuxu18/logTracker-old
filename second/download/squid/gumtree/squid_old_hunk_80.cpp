#if USE_SSL

    debugs(3, 1, "Initializing https proxy context");

    Config.ssl_client.sslContext = sslCreateClientContext(Config.ssl_client.cert, Config.ssl_client.key, Config.ssl_client.version, Config.ssl_client.cipher, Config.ssl_client.options, Config.ssl_client.flags, Config.ssl_client.cafile, Config.ssl_client.capath, Config.ssl_client.crlfile);

    {

        peer *p;

        for (p = Config.peers; p != NULL; p = p->next) {
            if (p->use_ssl) {
                debugs(3, 1, "Initializing cache_peer " << p->name << " SSL context");
                p->sslContext = sslCreateClientContext(p->sslcert, p->sslkey, p->sslversion, p->sslcipher, p->ssloptions, p->sslflags, p->sslcafile, p->sslcapath, p->sslcrlfile);
            }
        }
    }

    {

        http_port_list *s;

        for (s = Config.Sockaddr.http; s != NULL; s = (http_port_list *) s->next) {
            if (!s->cert && !s->key)
                continue;

            debugs(3, 1, "Initializing http_port " << s->http.s << " SSL context");

            s->staticSslContext.reset(
                sslCreateServerContext(s->cert, s->key,
                                       s->version, s->cipher, s->options, s->sslflags, s->clientca,
                                       s->cafile, s->capath, s->crlfile, s->dhfile,
                                       s->sslContextSessionId));

            Ssl::readCertAndPrivateKeyFromFiles(s->signingCert, s->signPkey, s->cert, s->key);
        }
    }

    {

        https_port_list *s;

        for (s = Config.Sockaddr.https; s != NULL; s = (https_port_list *) s->http.next) {
            debugs(3, 1, "Initializing https_port " << s->http.s << " SSL context");

            s->staticSslContext.reset(
                sslCreateServerContext(s->cert, s->key,
                                       s->version, s->cipher, s->options, s->sslflags, s->clientca,
                                       s->cafile, s->capath, s->crlfile, s->dhfile,
                                       s->sslContextSessionId));
        }
    }

#endif

    // prevent infinite fetch loops in the request parser
    // due to buffer full but not enough data recived to finish parse
    if (Config.maxRequestBufferSize <= Config.maxRequestHeaderSize) {
        fatalf("Client request buffer of %u bytes cannot hold a request with %u bytes of headers." \
               " Change client_request_buffer_max or request_header_max_size limits.",
               (uint32_t)Config.maxRequestBufferSize, (uint32_t)Config.maxRequestHeaderSize);
    }

    /*
     * disable client side request pipelining. There is a race with
     * Negotiate and NTLM when the client sends a second request on an
     * connection before the authenticate challenge is sent. With
     * pipelining OFF, the client may fail to authenticate, but squid's
     * state will be preserved.
     */
    if (Config.onoff.pipeline_prefetch) {
        AuthConfig *nego = AuthConfig::Find("Negotiate");
        AuthConfig *ntlm = AuthConfig::Find("NTLM");
        if ((nego && nego->active()) || (ntlm && ntlm->active())) {
            debugs(3, DBG_IMPORTANT, "WARNING: pipeline_prefetch breaks NTLM and Negotiate authentication. Forced OFF.");
            Config.onoff.pipeline_prefetch = 0;
        }
    }
}

/** Parse a line containing an obsolete directive.
 * To upgrade it where possible instead of just "Bungled config" for
 * directives which cannot be marked as simply aliases of the some name.
 * For example if the parameter order and content has changed.
 * Or if the directive has been completely removed.
 */
void
parse_obsolete(const char *name)
{
    // Directives which have been radically changed rather than removed
}

/* Parse a time specification from the config file.  Store the
 * result in 'tptr', after converting it to 'units' */
static void
parseTimeLine(time_t * tptr, const char *units)
{
    char *token;
    double d;
    time_t m;
    time_t u;

    if ((u = parseTimeUnits(units)) == 0)
        self_destruct();

    if ((token = strtok(NULL, w_space)) == NULL)
        self_destruct();

    d = xatof(token);
