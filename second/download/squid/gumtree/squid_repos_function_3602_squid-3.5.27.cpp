void
ConnStateData::kick()
{
    ConnStateData * conn = this; // XXX: Remove this diff minimization hack

    if (conn->pinning.pinned && !Comm::IsConnOpen(conn->pinning.serverConnection)) {
        debugs(33, 2, HERE << conn->clientConnection << " Connection was pinned but server side gone. Terminating client connection");
        conn->clientConnection->close();
        return;
    }

    /** \par
     * We are done with the response, and we are either still receiving request
     * body (early response!) or have already stopped receiving anything.
     *
     * If we are still receiving, then clientParseRequest() below will fail.
     * (XXX: but then we will call readNextRequest() which may succeed and
     * execute a smuggled request as we are not done with the current request).
     *
     * If we stopped because we got everything, then try the next request.
     *
     * If we stopped receiving because of an error, then close now to avoid
     * getting stuck and to prevent accidental request smuggling.
     */

    if (const char *reason = conn->stoppedReceiving()) {
        debugs(33, 3, HERE << "closing for earlier request error: " << reason);
        conn->clientConnection->close();
        return;
    }

    /** \par
     * Attempt to parse a request from the request buffer.
     * If we've been fed a pipelined request it may already
     * be in our read buffer.
     *
     \par
     * This needs to fall through - if we're unlucky and parse the _last_ request
     * from our read buffer we may never re-register for another client read.
     */

    if (conn->clientParseRequests()) {
        debugs(33, 3, HERE << conn->clientConnection << ": parsed next request from buffer");
    }

    /** \par
     * Either we need to kick-start another read or, if we have
     * a half-closed connection, kill it after the last request.
     * This saves waiting for half-closed connections to finished being
     * half-closed _AND_ then, sometimes, spending "Timeout" time in
     * the keepalive "Waiting for next request" state.
     */
    if (commIsHalfClosed(conn->clientConnection->fd) && (conn->getConcurrentRequestCount() == 0)) {
        debugs(33, 3, "ClientSocketContext::keepaliveNextRequest: half-closed client with no pending requests, closing");
        conn->clientConnection->close();
        return;
    }

    ClientSocketContext::Pointer deferredRequest;

    /** \par
     * At this point we either have a parsed request (which we've
     * kicked off the processing for) or not. If we have a deferred
     * request (parsed but deferred for pipeling processing reasons)
     * then look at processing it. If not, simply kickstart
     * another read.
     */

    if ((deferredRequest = conn->getCurrentContext()).getRaw()) {
        debugs(33, 3, HERE << conn->clientConnection << ": calling PushDeferredIfNeeded");
        ClientSocketContextPushDeferredIfNeeded(deferredRequest, conn);
    } else if (conn->flags.readMore) {
        debugs(33, 3, HERE << conn->clientConnection << ": calling conn->readNextRequest()");
        conn->readNextRequest();
    } else {
        // XXX: Can this happen? CONNECT tunnels have deferredRequest set.
        debugs(33, DBG_IMPORTANT, HERE << "abandoning " << conn->clientConnection);
    }
}