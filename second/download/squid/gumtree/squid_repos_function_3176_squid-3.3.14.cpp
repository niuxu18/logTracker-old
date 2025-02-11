void
internalStart(const Comm::ConnectionPointer &clientConn, HttpRequest * request, StoreEntry * entry)
{
    ErrorState *err;
    const char *upath = request->urlpath.termedBuf();
    debugs(76, 3, HERE << clientConn << " requesting '" << upath << "'");

    if (0 == strcmp(upath, "/squid-internal-dynamic/netdb")) {
        netdbBinaryExchange(entry);
    } else if (0 == strcmp(upath, "/squid-internal-periodic/store_digest")) {
#if USE_CACHE_DIGESTS
        const char *msgbuf = "This cache is currently building its digest.\n";
#else

        const char *msgbuf = "This cache does not support Cache Digests.\n";
#endif

        HttpReply *reply = new HttpReply;
        reply->setHeaders(HTTP_NOT_FOUND, "Not Found", "text/plain", strlen(msgbuf), squid_curtime, -2);
        entry->replaceHttpReply(reply);
        entry->append(msgbuf, strlen(msgbuf));
        entry->complete();
    } else if (0 == strncmp(upath, "/squid-internal-mgr/", 20)) {
        CacheManager::GetInstance()->Start(clientConn, request, entry);
    } else {
        debugObj(76, 1, "internalStart: unknown request:\n",
                 request, (ObjPackMethod) & httpRequestPack);
        err = new ErrorState(ERR_INVALID_REQ, HTTP_NOT_FOUND, request);
        errorAppendEntry(entry, err);
    }
}