void
ClientRequestContext::clientRedirectDone(const Helper::Reply &reply)
{
    HttpRequest *old_request = http->request;
    debugs(85, 5, HERE << "'" << http->uri << "' result=" << reply);
    assert(redirect_state == REDIRECT_PENDING);
    redirect_state = REDIRECT_DONE;

    // Put helper response Notes into the transaction state record (ALE) eventually
    // do it early to ensure that no matter what the outcome the notes are present.
    if (http->al != NULL)
        (void)SyncNotes(*http->al, *old_request);

    UpdateRequestNotes(http->getConn(), *old_request, reply.notes);

    switch (reply.result) {
    case Helper::TimedOut:
        if (Config.onUrlRewriteTimeout.action != toutActBypass) {
            http->calloutsError(ERR_GATEWAY_FAILURE, ERR_DETAIL_REDIRECTOR_TIMEDOUT);
            debugs(85, DBG_IMPORTANT, "ERROR: URL rewrite helper: Timedout");
        }
        break;

    case Helper::Unknown:
    case Helper::TT:
        // Handler in redirect.cc should have already mapped Unknown
        // IF it contained valid entry for the old URL-rewrite helper protocol
        debugs(85, DBG_IMPORTANT, "ERROR: URL rewrite helper returned invalid result code. Wrong helper? " << reply);
        break;

    case Helper::BrokenHelper:
        debugs(85, DBG_IMPORTANT, "ERROR: URL rewrite helper: " << reply);
        break;

    case Helper::Error:
        // no change to be done.
        break;

    case Helper::Okay: {
        // #1: redirect with a specific status code    OK status=NNN url="..."
        // #2: redirect with a default status code     OK url="..."
        // #3: re-write the URL                        OK rewrite-url="..."

        const char *statusNote = reply.notes.findFirst("status");
        const char *urlNote = reply.notes.findFirst("url");

        if (urlNote != NULL) {
            // HTTP protocol redirect to be done.

            // TODO: change default redirect status for appropriate requests
            // Squid defaults to 302 status for now for better compatibility with old clients.
            // HTTP/1.0 client should get 302 (Http::scFound)
            // HTTP/1.1 client contacting reverse-proxy should get 307 (Http::scTemporaryRedirect)
            // HTTP/1.1 client being diverted by forward-proxy should get 303 (Http::scSeeOther)
            Http::StatusCode status = Http::scFound;
            if (statusNote != NULL) {
                const char * result = statusNote;
                status = static_cast<Http::StatusCode>(atoi(result));
            }

            if (status == Http::scMovedPermanently
                    || status == Http::scFound
                    || status == Http::scSeeOther
                    || status == Http::scPermanentRedirect
                    || status == Http::scTemporaryRedirect) {
                http->redirect.status = status;
                http->redirect.location = xstrdup(urlNote);
                // TODO: validate the URL produced here is RFC 2616 compliant absolute URI
            } else {
                debugs(85, DBG_CRITICAL, "ERROR: URL-rewrite produces invalid " << status << " redirect Location: " << urlNote);
            }
        } else {
            // URL-rewrite wanted. Ew.
            urlNote = reply.notes.findFirst("rewrite-url");

            // prevent broken helpers causing too much damage. If old URL == new URL skip the re-write.
            if (urlNote != NULL && strcmp(urlNote, http->uri)) {
                URL tmpUrl;
                if (tmpUrl.parse(old_request->method, const_cast<char*>(urlNote))) {
                    HttpRequest *new_request = old_request->clone();
                    new_request->url = tmpUrl;
                    debugs(61, 2, "URL-rewriter diverts URL from " << old_request->effectiveRequestUri() << " to " << new_request->effectiveRequestUri());

                    // update the new request to flag the re-writing was done on it
                    new_request->flags.redirected = true;

                    // unlink bodypipe from the old request. Not needed there any longer.
                    if (old_request->body_pipe != NULL) {
                        old_request->body_pipe = NULL;
                        debugs(61,2, HERE << "URL-rewriter diverts body_pipe " << new_request->body_pipe <<
                               " from request " << old_request << " to " << new_request);
                    }

                    // update the current working ClientHttpRequest fields
                    xfree(http->uri);
                    http->uri = SBufToCstring(new_request->effectiveRequestUri());
                    HTTPMSGUNLOCK(old_request);
                    http->request = new_request;
                    HTTPMSGLOCK(http->request);
                } else {
                    debugs(85, DBG_CRITICAL, "ERROR: URL-rewrite produces invalid request: " <<
                           old_request->method << " " << urlNote << " " << old_request->http_ver);
                }
            }
        }
    }
    break;
    }

    /* FIXME PIPELINE: This is innacurate during pipelining */

    if (http->getConn() != NULL && Comm::IsConnOpen(http->getConn()->clientConnection))
        fd_note(http->getConn()->clientConnection->fd, http->uri);

    assert(http->uri);

    http->doCallouts();
}