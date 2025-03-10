void Adaptation::Icap::ModXact::makeRequestHeaders(MemBuf &buf)
{
    char ntoabuf[MAX_IPSTRLEN];
    /*
     * XXX These should use HttpHdr interfaces instead of Printfs
     */
    const Adaptation::ServiceConfig &s = service().cfg();
    buf.appendf("%s " SQUIDSTRINGPH " ICAP/1.0\r\n", s.methodStr(), SQUIDSTRINGPRINT(s.uri));
    buf.appendf("Host: " SQUIDSTRINGPH ":%d\r\n", SQUIDSTRINGPRINT(s.host), s.port);
    buf.appendf("Date: %s\r\n", mkrfc1123(squid_curtime));

    if (!TheConfig.reuse_connections)
        buf.appendf("Connection: close\r\n");

    const HttpRequest *request = &virginRequest();

    // we must forward "Proxy-Authenticate" and "Proxy-Authorization"
    // as ICAP headers.
    if (virgin.header->header.has(Http::HdrType::PROXY_AUTHENTICATE)) {
        String vh=virgin.header->header.getById(Http::HdrType::PROXY_AUTHENTICATE);
        buf.appendf("Proxy-Authenticate: " SQUIDSTRINGPH "\r\n",SQUIDSTRINGPRINT(vh));
    }

    if (virgin.header->header.has(Http::HdrType::PROXY_AUTHORIZATION)) {
        String vh=virgin.header->header.getById(Http::HdrType::PROXY_AUTHORIZATION);
        buf.appendf("Proxy-Authorization: " SQUIDSTRINGPH "\r\n", SQUIDSTRINGPRINT(vh));
    } else if (request->extacl_user.size() > 0 && request->extacl_passwd.size() > 0) {
        struct base64_encode_ctx ctx;
        base64_encode_init(&ctx);
        uint8_t base64buf[base64_encode_len(MAX_LOGIN_SZ)];
        size_t resultLen = base64_encode_update(&ctx, base64buf, request->extacl_user.size(), reinterpret_cast<const uint8_t*>(request->extacl_user.rawBuf()));
        resultLen += base64_encode_update(&ctx, base64buf+resultLen, 1, reinterpret_cast<const uint8_t*>(":"));
        resultLen += base64_encode_update(&ctx, base64buf+resultLen, request->extacl_passwd.size(), reinterpret_cast<const uint8_t*>(request->extacl_passwd.rawBuf()));
        resultLen += base64_encode_final(&ctx, base64buf+resultLen);
        buf.appendf("Proxy-Authorization: Basic %.*s\r\n", (int)resultLen, base64buf);
    }

    // share the cross-transactional database records if needed
    if (Adaptation::Config::masterx_shared_name) {
        Adaptation::History::Pointer ah = request->adaptHistory(false);
        if (ah != NULL) {
            String name, value;
            if (ah->getXxRecord(name, value)) {
                buf.appendf(SQUIDSTRINGPH ": " SQUIDSTRINGPH "\r\n", SQUIDSTRINGPRINT(name), SQUIDSTRINGPRINT(value));
            }
        }
    }

    buf.append("Encapsulated: ", 14);

    MemBuf httpBuf;

    httpBuf.init();

    // build HTTP request header, if any
    ICAP::Method m = s.method;

    // to simplify, we could assume that request is always available

    if (request) {
        if (ICAP::methodRespmod == m)
            encapsulateHead(buf, "req-hdr", httpBuf, request);
        else if (ICAP::methodReqmod == m)
            encapsulateHead(buf, "req-hdr", httpBuf, virgin.header);
    }

    if (ICAP::methodRespmod == m)
        if (const HttpMsg *prime = virgin.header)
            encapsulateHead(buf, "res-hdr", httpBuf, prime);

    if (!virginBody.expected())
        buf.appendf("null-body=%d", (int) httpBuf.contentSize());
    else if (ICAP::methodReqmod == m)
        buf.appendf("req-body=%d", (int) httpBuf.contentSize());
    else
        buf.appendf("res-body=%d", (int) httpBuf.contentSize());

    buf.append(ICAP::crlf, 2); // terminate Encapsulated line

    if (preview.enabled()) {
        buf.appendf("Preview: %d\r\n", (int)preview.ad());
        if (!virginBody.expected()) // there is no body to preview
            finishNullOrEmptyBodyPreview(httpBuf);
    }

    makeAllowHeader(buf);

    if (TheConfig.send_client_ip && request) {
        Ip::Address client_addr;
#if FOLLOW_X_FORWARDED_FOR
        if (TheConfig.use_indirect_client) {
            client_addr = request->indirect_client_addr;
        } else
#endif
            client_addr = request->client_addr;
        if (!client_addr.isAnyAddr() && !client_addr.isNoAddr())
            buf.appendf("X-Client-IP: %s\r\n", client_addr.toStr(ntoabuf,MAX_IPSTRLEN));
    }

    if (TheConfig.send_username && request)
        makeUsernameHeader(request, buf);

    // Adaptation::Config::metaHeaders
    typedef Notes::iterator ACAMLI;
    for (ACAMLI i = Adaptation::Config::metaHeaders.begin(); i != Adaptation::Config::metaHeaders.end(); ++i) {
        HttpRequest *r = virgin.cause ?
                         virgin.cause : dynamic_cast<HttpRequest*>(virgin.header);
        Must(r);

        HttpReply *reply = dynamic_cast<HttpReply*>(virgin.header);

        if (const char *value = (*i)->match(r, reply, alMaster)) {
            buf.appendf("%s: %s\r\n", (*i)->key.termedBuf(), value);
            Adaptation::History::Pointer ah = request->adaptHistory(false);
            if (ah != NULL) {
                if (ah->metaHeaders == NULL)
                    ah->metaHeaders = new NotePairs;
                if (!ah->metaHeaders->hasPair((*i)->key.termedBuf(), value))
                    ah->metaHeaders->add((*i)->key.termedBuf(), value);
            }
        }
    }

    // fprintf(stderr, "%s\n", buf.content());

    buf.append(ICAP::crlf, 2); // terminate ICAP header

    // fill icapRequest for logging
    Must(icapRequest->parseCharBuf(buf.content(), buf.contentSize()));

    // start ICAP request body with encapsulated HTTP headers
    buf.append(httpBuf.content(), httpBuf.contentSize());

    httpBuf.clean();
}