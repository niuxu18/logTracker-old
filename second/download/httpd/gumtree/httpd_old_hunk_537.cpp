        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
                     "Changed client verification locations will force "
                     "renegotiation");
    }
#endif /* HAVE_SSL_SET_CERT_STORE */

    /* 
     * SSL renegotiations in conjunction with HTTP
     * requests using the POST method are not supported.
     *
     * Background:
     *
     * 1. When the client sends a HTTP/HTTPS request, Apache's core code
     * reads only the request line ("METHOD /path HTTP/x.y") and the
     * attached MIME headers ("Foo: bar") up to the terminating line ("CR
     * LF"). An attached request body (for instance the data of a POST
     * method) is _NOT_ read. Instead it is read by mod_cgi's content
     * handler and directly passed to the CGI script.
     *
     * 2. mod_ssl supports per-directory re-configuration of SSL parameters.
     * This is implemented by performing an SSL renegotiation of the
     * re-configured parameters after the request is read, but before the
     * response is sent. In more detail: the renegotiation happens after the
     * request line and MIME headers were read, but _before_ the attached
     * request body is read. The reason simply is that in the HTTP protocol
     * usually there is no acknowledgment step between the headers and the
     * body (there is the 100-continue feature and the chunking facility
     * only), so Apache has no API hook for this step.
     *
     * 3. the problem now occurs when the client sends a POST request for
     * URL /foo via HTTPS the server and the server has SSL parameters
     * re-configured on a per-URL basis for /foo. Then mod_ssl has to
     * perform an SSL renegotiation after the request was read and before
     * the response is sent. But the problem is the pending POST body data
     * in the receive buffer of SSL (which Apache still has not read - it's
     * pending until mod_cgi sucks it in). When mod_ssl now tries to perform
     * the renegotiation the pending data leads to an I/O error.
     *
     * Solution Idea:
     *
     * There are only two solutions: Either to simply state that POST
     * requests to URLs with SSL re-configurations are not allowed, or to
     * renegotiate really after the _complete_ request (i.e. including
     * the POST body) was read. Obviously the latter would be preferred,
     * but it cannot be done easily inside Apache, because as already
     * mentioned, there is no API step between the body reading and the body
     * processing. And even when we mod_ssl would hook directly into the
     * loop of mod_cgi, we wouldn't solve the problem for other handlers, of
     * course. So the only general solution is to suck in the pending data
     * of the request body from the OpenSSL BIO into the Apache BUFF. Then
     * the renegotiation can be done and after this step Apache can proceed
     * processing the request as before.
     *
     * Solution Implementation:
     *
     * We cannot simply suck in the data via an SSL_read-based loop because of
     * HTTP chunking. Instead we _have_ to use the Apache API for this step which
     * is aware of HTTP chunking. So the trick is to suck in the pending request
     * data via the Apache API (which uses Apache's BUFF code and in the
     * background mod_ssl's I/O glue code) and re-inject it later into the Apache
     * BUFF code again. This way the data flows twice through the Apache BUFF, of
     * course. But this way the solution doesn't depend on any Apache specifics
     * and is fully transparent to Apache modules.
     *
     * !! BUT ALL THIS IS STILL NOT RE-IMPLEMENTED FOR APACHE 2.0 !!
     */
    if (renegotiate && !renegotiate_quick && (r->method_number == M_POST)) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                     "SSL Re-negotiation in conjunction "
                     "with POST method not supported! "
                     "hint: try SSLOptions +OptRenegotiate");

        return HTTP_METHOD_NOT_ALLOWED;
    }

    /*
     * now do the renegotiation if anything was actually reconfigured
     */
    if (renegotiate) {
