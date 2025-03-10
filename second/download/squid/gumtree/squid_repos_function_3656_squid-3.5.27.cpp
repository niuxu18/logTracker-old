static void
httpsEstablish(ConnStateData *connState,  SSL_CTX *sslContext, Ssl::BumpMode bumpMode)
{
    SSL *ssl = NULL;
    assert(connState);
    const Comm::ConnectionPointer &details = connState->clientConnection;

    if (sslContext && !(ssl = httpsCreate(details, sslContext)))
        return;

    typedef CommCbMemFunT<ConnStateData, CommTimeoutCbParams> TimeoutDialer;
    AsyncCall::Pointer timeoutCall = JobCallback(33, 5, TimeoutDialer,
                                     connState, ConnStateData::requestTimeout);
    commSetConnTimeout(details, Config.Timeout.request, timeoutCall);

    if (ssl)
        Comm::SetSelect(details->fd, COMM_SELECT_READ, clientNegotiateSSL, connState, 0);
    else {
        char buf[MAX_IPSTRLEN];
        assert(bumpMode != Ssl::bumpNone && bumpMode != Ssl::bumpEnd);
        HttpRequest::Pointer fakeRequest(new HttpRequest);
        fakeRequest->SetHost(details->local.toStr(buf, sizeof(buf)));
        fakeRequest->port = details->local.port();
        fakeRequest->clientConnectionManager = connState;
        fakeRequest->client_addr = connState->clientConnection->remote;
#if FOLLOW_X_FORWARDED_FOR
        fakeRequest->indirect_client_addr = connState->clientConnection->remote;
#endif
        fakeRequest->my_addr = connState->clientConnection->local;
        fakeRequest->flags.interceptTproxy = ((connState->clientConnection->flags & COMM_TRANSPARENT) != 0 ) ;
        fakeRequest->flags.intercepted = ((connState->clientConnection->flags & COMM_INTERCEPTION) != 0);
        fakeRequest->myportname = connState->port->name;
        if (fakeRequest->flags.interceptTproxy) {
            if (Config.accessList.spoof_client_ip) {
                ACLFilledChecklist checklist(Config.accessList.spoof_client_ip, fakeRequest.getRaw(), NULL);
                fakeRequest->flags.spoofClientIp = (checklist.fastCheck() == ACCESS_ALLOWED);
            } else
                fakeRequest->flags.spoofClientIp = true;
        } else
            fakeRequest->flags.spoofClientIp = false;
        debugs(33, 4, HERE << details << " try to generate a Dynamic SSL CTX");
        connState->switchToHttps(fakeRequest.getRaw(), bumpMode);
    }
}