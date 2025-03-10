void
clientReplyContext::purgeDoPurgeHead(StoreEntry *newEntry)
{
    if (newEntry && !newEntry->isNull()) {
        debugs(88, 4, "clientPurgeRequest: HEAD '" << newEntry->url() << "'" );
#if USE_HTCP
        neighborsHtcpClear(newEntry, NULL, http->request, HttpRequestMethod(METHOD_HEAD), HTCP_CLR_PURGE);
#endif
        newEntry->release();
        purgeStatus = HTTP_OK;
    }

    /* And for Vary, release the base URI if none of the headers was included in the request */

    if (http->request->vary_headers
            && !strstr(http->request->vary_headers, "=")) {
        StoreEntry *entry = storeGetPublic(urlCanonical(http->request), METHOD_GET);

        if (entry) {
            debugs(88, 4, "clientPurgeRequest: Vary GET '" << entry->url() << "'" );
#if USE_HTCP
            neighborsHtcpClear(entry, NULL, http->request, HttpRequestMethod(METHOD_GET), HTCP_CLR_PURGE);
#endif
            entry->release();
            purgeStatus = HTTP_OK;
        }

        entry = storeGetPublic(urlCanonical(http->request), METHOD_HEAD);

        if (entry) {
            debugs(88, 4, "clientPurgeRequest: Vary HEAD '" << entry->url() << "'" );
#if USE_HTCP
            neighborsHtcpClear(entry, NULL, http->request, HttpRequestMethod(METHOD_HEAD), HTCP_CLR_PURGE);
#endif
            entry->release();
            purgeStatus = HTTP_OK;
        }
    }

    /*
     * Make a new entry to hold the reply to be written
     * to the client.
     */
    /* FIXME: This doesn't need to go through the store. Simply
     * push down the client chain
     */
    createStoreEntry(http->request->method, RequestFlags());

    triggerInitialStoreRead();

    HttpReply *rep = new HttpReply;
    rep->setHeaders(purgeStatus, NULL, NULL, 0, 0, -1);
    http->storeEntry()->replaceHttpReply(rep);
    http->storeEntry()->complete();
}