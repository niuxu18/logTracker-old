HttpRequest *
HttpRequest::clone() const
{
    HttpRequest *copy = new HttpRequest(method, url.getScheme(), urlpath.termedBuf());
    // TODO: move common cloning clone to Msg::copyTo() or copy ctor
    copy->header.append(&header);
    copy->hdrCacheInit();
    copy->hdr_sz = hdr_sz;
    copy->http_ver = http_ver;
    copy->pstate = pstate; // TODO: should we assert a specific state here?
    copy->body_pipe = body_pipe;

    strncpy(copy->login, login, sizeof(login)); // MAX_LOGIN_SZ
    strncpy(copy->host, host, sizeof(host)); // SQUIDHOSTNAMELEN
    copy->host_addr = host_addr;

    copy->port = port;
    // urlPath handled in ctor
    copy->canonical = canonical ? xstrdup(canonical) : NULL;

    // range handled in hdrCacheInit()
    copy->ims = ims;
    copy->imslen = imslen;
    copy->hier = hier; // Is it safe to copy? Should we?

    copy->errType = errType;

    // XXX: what to do with copy->peer_login?

    copy->lastmod = lastmod;
    copy->etag = etag;
    copy->vary_headers = vary_headers;
    // XXX: what to do with copy->peer_domain?

    copy->tag = tag;
    copy->extacl_log = extacl_log;
    copy->extacl_message = extacl_message;

    const bool inheritWorked = copy->inheritProperties(this);
    assert(inheritWorked);

    return copy;
}