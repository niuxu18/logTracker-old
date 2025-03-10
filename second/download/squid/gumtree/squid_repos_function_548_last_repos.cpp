void
whoisStart(FwdState * fwd)
{
    WhoisState *p = new WhoisState;
    p->request = fwd->request;
    p->entry = fwd->entry;
    p->fwd = fwd;
    p->dataWritten = false;

    p->entry->lock("whoisStart");
    comm_add_close_handler(fwd->serverConnection()->fd, whoisClose, p);

    size_t l = p->request->url.path().length() + 3;
    char *buf = (char *)xmalloc(l);

    const SBuf str_print = p->request->url.path().substr(1);
    snprintf(buf, l, SQUIDSBUFPH "\r\n", SQUIDSBUFPRINT(str_print));

    AsyncCall::Pointer writeCall = commCbCall(5,5, "whoisWriteComplete",
                                   CommIoCbPtrFun(whoisWriteComplete, p));
    Comm::Write(fwd->serverConnection(), buf, strlen(buf), writeCall, NULL);
    AsyncCall::Pointer readCall = commCbCall(5,4, "whoisReadReply",
                                  CommIoCbPtrFun(whoisReadReply, p));
    comm_read(fwd->serverConnection(), p->buf, BUFSIZ, readCall);
    AsyncCall::Pointer timeoutCall = commCbCall(5, 4, "whoisTimeout",
                                     CommTimeoutCbPtrFun(whoisTimeout, p));
    commSetConnTimeout(fwd->serverConnection(), Config.Timeout.read, timeoutCall);
}