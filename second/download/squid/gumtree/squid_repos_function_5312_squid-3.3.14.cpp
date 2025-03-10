void
Auth::Digest::UserRequest::addAuthenticationInfoTrailer(HttpReply * rep, int accel)
{
    int type;

    if (!auth_user_request)
        return;

    /* has the header already been send? */
    if (flags.authinfo_sent)
        return;

    /* don't add to authentication error pages */
    if ((!accel && rep->sline.status == HTTP_PROXY_AUTHENTICATION_REQUIRED)
            || (accel && rep->sline.status == HTTP_UNAUTHORIZED))
        return;

    type = accel ? HDR_AUTHENTICATION_INFO : HDR_PROXY_AUTHENTICATION_INFO;

    if ((static_cast<Auth::Digest::Config*>(digestScheme::GetInstance()->getConfig())->authenticate) && authDigestNonceLastRequest(nonce)) {
        debugs(29, 9, HERE << "Sending type:" << type << " header: 'nextnonce=\"" << authenticateDigestNonceNonceb64(nonce) << "\"");
        httpTrailerPutStrf(&rep->header, type, "nextnonce=\"%s\"", authenticateDigestNonceNonceb64(nonce));
    }
}