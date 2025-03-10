void
Format::Format::assemble(MemBuf &mb, const AccessLogEntry::Pointer &al, int logSequenceNumber) const
{
    char tmp[1024];
    String sb;

    for (Token *fmt = format; fmt != NULL; fmt = fmt->next) {	/* for each token */
        const char *out = NULL;
        int quote = 0;
        long int outint = 0;
        int doint = 0;
        int dofree = 0;
        int64_t outoff = 0;
        int dooff = 0;

        switch (fmt->type) {

        case LFT_NONE:
            out = "";
            break;

        case LFT_STRING:
            out = fmt->data.string;
            break;

        case LFT_CLIENT_IP_ADDRESS:
            al->getLogClientIp(tmp, sizeof(tmp));
            out = tmp;
            break;

        case LFT_CLIENT_FQDN:
            if (al->cache.caddr.IsAnyAddr()) // e.g., ICAP OPTIONS lack client
                out = "-";
            else
                out = fqdncache_gethostbyaddr(al->cache.caddr, FQDN_LOOKUP_IF_MISS);
            if (!out) {
                out = al->cache.caddr.NtoA(tmp,1024);
            }

            break;

        case LFT_CLIENT_PORT:
            if (al->request) {
                outint = al->request->client_addr.GetPort();
                doint = 1;
            }
            break;

        case LFT_CLIENT_EUI:
#if USE_SQUID_EUI
            // TODO make the ACL checklist have a direct link to any TCP details.
            if (al->request && al->request->clientConnectionManager.valid() && al->request->clientConnectionManager->clientConnection != NULL) {
                if (al->request->clientConnectionManager->clientConnection->remote.IsIPv4())
                    al->request->clientConnectionManager->clientConnection->remoteEui48.encode(tmp, 1024);
                else
                    al->request->clientConnectionManager->clientConnection->remoteEui64.encode(tmp, 1024);
                out = tmp;
            }
#else
            out = "-";
#endif
            break;

        case LFT_SERVER_IP_ADDRESS:
            if (al->hier.tcpServer != NULL) {
                out = al->hier.tcpServer->remote.NtoA(tmp,sizeof(tmp));
            }
            break;

        case LFT_SERVER_FQDN_OR_PEER_NAME:
            out = al->hier.host;
            break;

        case LFT_SERVER_PORT:
            if (al->hier.tcpServer != NULL) {
                outint = al->hier.tcpServer->remote.GetPort();
                doint = 1;
            }
            break;

        case LFT_LOCAL_LISTENING_IP: {
            // avoid logging a dash if we have reliable info
            const bool interceptedAtKnownPort = al->request ?
                                                (al->request->flags.spoofClientIp ||
                                                 al->request->flags.intercepted) && al->cache.port :
                                                false;
            if (interceptedAtKnownPort) {
                const bool portAddressConfigured = !al->cache.port->s.IsAnyAddr();
                if (portAddressConfigured)
                    out = al->cache.port->s.NtoA(tmp, sizeof(tmp));
            } else if (al->tcpClient != NULL)
                out = al->tcpClient->local.NtoA(tmp, sizeof(tmp));
        }
        break;

        case LFT_CLIENT_LOCAL_IP:
            if (al->tcpClient != NULL) {
                out = al->tcpClient->local.NtoA(tmp,sizeof(tmp));
            }
            break;

        case LFT_LOCAL_LISTENING_PORT:
            if (al->cache.port) {
                outint = al->cache.port->s.GetPort();
                doint = 1;
            }
            break;

        case LFT_CLIENT_LOCAL_PORT:
            if (al->tcpClient != NULL) {
                outint = al->tcpClient->local.GetPort();
                doint = 1;
            }
            break;

        case LFT_SERVER_LOCAL_IP_OLD_27:
        case LFT_SERVER_LOCAL_IP:
            if (al->hier.tcpServer != NULL) {
                out = al->hier.tcpServer->local.NtoA(tmp,sizeof(tmp));
            }
            break;

        case LFT_SERVER_LOCAL_PORT:
            if (al->hier.tcpServer != NULL) {
                outint = al->hier.tcpServer->local.GetPort();
                doint = 1;
            }

            break;

        case LFT_TIME_SECONDS_SINCE_EPOCH:
            // some platforms store time in 32-bit, some 64-bit...
            outoff = static_cast<int64_t>(current_time.tv_sec);
            dooff = 1;
            break;

        case LFT_TIME_SUBSECOND:
            outint = current_time.tv_usec / fmt->divisor;
            doint = 1;
            break;

        case LFT_TIME_LOCALTIME:

        case LFT_TIME_GMT: {
            const char *spec;

            struct tm *t;
            spec = fmt->data.timespec;

            if (fmt->type == LFT_TIME_LOCALTIME) {
                if (!spec)
                    spec = "%d/%b/%Y:%H:%M:%S %z";
                t = localtime(&squid_curtime);
            } else {
                if (!spec)
                    spec = "%d/%b/%Y:%H:%M:%S";

                t = gmtime(&squid_curtime);
            }

            strftime(tmp, sizeof(tmp), spec, t);

            out = tmp;
        }

        break;

        case LFT_TIME_TO_HANDLE_REQUEST:
            outint = al->cache.msec;
            doint = 1;
            break;

        case LFT_PEER_RESPONSE_TIME:
            if (al->hier.peer_response_time < 0) {
                out = "-";
            } else {
                outoff = al->hier.peer_response_time;
                dooff = 1;
            }
            break;

        case LFT_TOTAL_SERVER_SIDE_RESPONSE_TIME:
            if (al->hier.total_response_time < 0) {
                out = "-";
            } else {
                outoff = al->hier.total_response_time;
                dooff = 1;
            }
            break;

        case LFT_DNS_WAIT_TIME:
            if (al->request && al->request->dnsWait >= 0) {
                outint = al->request->dnsWait;
                doint = 1;
            }
            break;

        case LFT_REQUEST_HEADER:

            if (al->request)
                sb = al->request->header.getByName(fmt->data.header.header);

            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_ADAPTED_REQUEST_HEADER:

            if (al->adapted_request)
                sb = al->adapted_request->header.getByName(fmt->data.header.header);

            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_REPLY_HEADER:
            if (al->reply)
                sb = al->reply->header.getByName(fmt->data.header.header);

            out = sb.termedBuf();

            quote = 1;

            break;

#if USE_ADAPTATION
        case LFT_ADAPTATION_SUM_XACT_TIMES:
            if (al->request) {
                Adaptation::History::Pointer ah = al->request->adaptHistory();
                if (ah != NULL)
                    ah->sumLogString(fmt->data.string, sb);
                out = sb.termedBuf();
            }
            break;

        case LFT_ADAPTATION_ALL_XACT_TIMES:
            if (al->request) {
                Adaptation::History::Pointer ah = al->request->adaptHistory();
                if (ah != NULL)
                    ah->allLogString(fmt->data.string, sb);
                out = sb.termedBuf();
            }
            break;

        case LFT_ADAPTATION_LAST_HEADER:
            if (al->request) {
                const Adaptation::History::Pointer ah = al->request->adaptHistory();
                if (ah != NULL) // XXX: add adapt::<all_h but use lastMeta here
                    sb = ah->allMeta.getByName(fmt->data.header.header);
            }

            // XXX: here and elsewhere: move such code inside the if guard
            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_ADAPTATION_LAST_HEADER_ELEM:
            if (al->request) {
                const Adaptation::History::Pointer ah = al->request->adaptHistory();
                if (ah != NULL) // XXX: add adapt::<all_h but use lastMeta here
                    sb = ah->allMeta.getByNameListMember(fmt->data.header.header, fmt->data.header.element, fmt->data.header.separator);
            }

            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_ADAPTATION_LAST_ALL_HEADERS:
            out = al->adapt.last_meta;

            quote = 1;

            break;
#endif

#if ICAP_CLIENT
        case LFT_ICAP_ADDR:
            if (!out)
                out = al->icap.hostAddr.NtoA(tmp,1024);
            break;

        case LFT_ICAP_SERV_NAME:
            out = al->icap.serviceName.termedBuf();
            break;

        case LFT_ICAP_REQUEST_URI:
            out = al->icap.reqUri.termedBuf();
            break;

        case LFT_ICAP_REQUEST_METHOD:
            out = Adaptation::Icap::ICAP::methodStr(al->icap.reqMethod);
            break;

        case LFT_ICAP_BYTES_SENT:
            outoff = al->icap.bytesSent;
            dooff = 1;
            break;

        case LFT_ICAP_BYTES_READ:
            outoff = al->icap.bytesRead;
            dooff = 1;
            break;

        case LFT_ICAP_BODY_BYTES_READ:
            if (al->icap.bodyBytesRead >= 0) {
                outoff = al->icap.bodyBytesRead;
                dooff = 1;
            }
            // else if icap.bodyBytesRead < 0, we do not have any http data,
            // so just print a "-" (204 responses etc)
            break;

        case LFT_ICAP_REQ_HEADER:
            if (NULL != al->icap.request) {
                sb = al->icap.request->header.getByName(fmt->data.header.header);
                out = sb.termedBuf();
                quote = 1;
            }
            break;

        case LFT_ICAP_REQ_HEADER_ELEM:
            if (al->icap.request)
                sb = al->icap.request->header.getByNameListMember(fmt->data.header.header, fmt->data.header.element, fmt->data.header.separator);

            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_ICAP_REQ_ALL_HEADERS:
            if (al->icap.request) {
                HttpHeaderPos pos = HttpHeaderInitPos;
                while (const HttpHeaderEntry *e = al->icap.request->header.getEntry(&pos)) {
                    sb.append(e->name);
                    sb.append(": ");
                    sb.append(e->value);
                    sb.append("\r\n");
                }
                out = sb.termedBuf();
                quote = 1;
            }
            break;

        case LFT_ICAP_REP_HEADER:
            if (NULL != al->icap.reply) {
                sb = al->icap.reply->header.getByName(fmt->data.header.header);
                out = sb.termedBuf();
                quote = 1;
            }
            break;

        case LFT_ICAP_REP_HEADER_ELEM:
            if (NULL != al->icap.reply)
                sb = al->icap.reply->header.getByNameListMember(fmt->data.header.header, fmt->data.header.element, fmt->data.header.separator);

            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_ICAP_REP_ALL_HEADERS:
            if (al->icap.reply) {
                HttpHeaderPos pos = HttpHeaderInitPos;
                while (const HttpHeaderEntry *e = al->icap.reply->header.getEntry(&pos)) {
                    sb.append(e->name);
                    sb.append(": ");
                    sb.append(e->value);
                    sb.append("\r\n");
                }
                out = sb.termedBuf();
                quote = 1;
            }
            break;

        case LFT_ICAP_TR_RESPONSE_TIME:
            outint = al->icap.trTime;
            doint = 1;
            break;

        case LFT_ICAP_IO_TIME:
            outint = al->icap.ioTime;
            doint = 1;
            break;

        case LFT_ICAP_STATUS_CODE:
            outint = al->icap.resStatus;
            doint  = 1;
            break;

        case LFT_ICAP_OUTCOME:
            out = al->icap.outcome;
            break;

        case LFT_ICAP_TOTAL_TIME:
            outint = al->icap.processingTime;
            doint = 1;
            break;
#endif
        case LFT_REQUEST_HEADER_ELEM:
            if (al->request)
                sb = al->request->header.getByNameListMember(fmt->data.header.header, fmt->data.header.element, fmt->data.header.separator);

            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_ADAPTED_REQUEST_HEADER_ELEM:
            if (al->adapted_request)
                sb = al->adapted_request->header.getByNameListMember(fmt->data.header.header, fmt->data.header.element, fmt->data.header.separator);

            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_REPLY_HEADER_ELEM:
            if (al->reply)
                sb = al->reply->header.getByNameListMember(fmt->data.header.header, fmt->data.header.element, fmt->data.header.separator);

            out = sb.termedBuf();

            quote = 1;

            break;

        case LFT_REQUEST_ALL_HEADERS:
            out = al->headers.request;

            quote = 1;

            break;

        case LFT_ADAPTED_REQUEST_ALL_HEADERS:
            out = al->headers.adapted_request;

            quote = 1;

            break;

        case LFT_REPLY_ALL_HEADERS:
            out = al->headers.reply;

            quote = 1;

            break;

        case LFT_USER_NAME:
#if USE_AUTH
            if (al->request && al->request->auth_user_request != NULL)
                out = strOrNull(al->request->auth_user_request->username());
#endif
            if (!out)
                out = strOrNull(al->cache.extuser);
#if USE_SSL
            if (!out)
                out = strOrNull(al->cache.ssluser);
#endif
            if (!out)
                out = strOrNull(al->cache.rfc931);
            break;

        case LFT_USER_LOGIN:
#if USE_AUTH
            if (al->request && al->request->auth_user_request != NULL)
                out = strOrNull(al->request->auth_user_request->username());
#endif
            break;

        case LFT_USER_IDENT:
            out = strOrNull(al->cache.rfc931);
            break;

        case LFT_USER_EXTERNAL:
            out = strOrNull(al->cache.extuser);
            break;

            /* case LFT_USER_REALM: */
            /* case LFT_USER_SCHEME: */

            // the fmt->type can not be LFT_HTTP_SENT_STATUS_CODE_OLD_30
            // but compiler complains if ommited
        case LFT_HTTP_SENT_STATUS_CODE_OLD_30:
        case LFT_HTTP_SENT_STATUS_CODE:
            outint = al->http.code;

            doint = 1;

            break;

        case LFT_HTTP_RECEIVED_STATUS_CODE:
            if (al->hier.peer_reply_status == HTTP_STATUS_NONE) {
                out = "-";
            } else {
                outint = al->hier.peer_reply_status;
                doint = 1;
            }
            break;
            /* case LFT_HTTP_STATUS:
             *           out = statusline->text;
             *     quote = 1;
             *     break;
             */
        case LFT_HTTP_BODY_BYTES_READ:
            if (al->hier.bodyBytesRead >= 0) {
                outoff = al->hier.bodyBytesRead;
                dooff = 1;
            }
            // else if hier.bodyBytesRead < 0 we did not have any data exchange with
            // a peer server so just print a "-" (eg requests served from cache,
            // or internal error messages).
            break;

        case LFT_SQUID_STATUS:
            if (al->http.timedout || al->http.aborted) {
                snprintf(tmp, sizeof(tmp), "%s%s", log_tags[al->cache.code],
                         al->http.statusSfx());
                out = tmp;
            } else {
                out = log_tags[al->cache.code];
            }

            break;

        case LFT_SQUID_ERROR:
            if (al->request && al->request->errType != ERR_NONE)
                out = errorPageName(al->request->errType);
            break;

        case LFT_SQUID_ERROR_DETAIL:
#if USE_SSL
            if (al->request && al->request->errType == ERR_SECURE_CONNECT_FAIL) {
                if (! (out = Ssl::GetErrorName(al->request->errDetail))) {
                    snprintf(tmp, sizeof(tmp), "SSL_ERR=%d", al->request->errDetail);
                    out = tmp;
                }
            } else
#endif
                if (al->request && al->request->errDetail != ERR_DETAIL_NONE) {
                    if (al->request->errDetail > ERR_DETAIL_START  &&
                            al->request->errDetail < ERR_DETAIL_MAX)
                        out = errorDetailName(al->request->errDetail);
                    else {
                        if (al->request->errDetail >= ERR_DETAIL_EXCEPTION_START)
                            snprintf(tmp, sizeof(tmp), "%s=0x%X",
                                     errorDetailName(al->request->errDetail), (uint32_t) al->request->errDetail);
                        else
                            snprintf(tmp, sizeof(tmp), "%s=%d",
                                     errorDetailName(al->request->errDetail), al->request->errDetail);
                        out = tmp;
                    }
                }
            break;

        case LFT_SQUID_HIERARCHY:
            if (al->hier.ping.timedout)
                mb.append("TIMEOUT_", 8);

            out = hier_code_str[al->hier.code];

            break;

        case LFT_MIME_TYPE:
            out = al->http.content_type;

            break;

        case LFT_CLIENT_REQ_METHOD:
            if (al->request) {
                out = al->request->method.image();
                quote = 1;
            }
            break;

        case LFT_CLIENT_REQ_URI:
            // original client URI
            if (al->request) {
                out = urlCanonical(al->request);
                quote = 1;
            }
            break;

        case LFT_REQUEST_URLPATH_OLD_31:
        case LFT_CLIENT_REQ_URLPATH:
            if (al->request) {
                out = al->request->urlpath.termedBuf();
                quote = 1;
            }
            break;

        case LFT_CLIENT_REQ_VERSION:
            if (al->request) {
                snprintf(tmp, sizeof(tmp), "%d.%d", (int) al->request->http_ver.major, (int) al->request->http_ver.minor);
                out = tmp;
            }
            break;

        case LFT_REQUEST_METHOD:
            out = al->_private.method_str;
            break;

        case LFT_REQUEST_URI:
            out = al->url;
            break;

        case LFT_REQUEST_VERSION_OLD_2X:
        case LFT_REQUEST_VERSION:
            snprintf(tmp, sizeof(tmp), "%d.%d", (int) al->http.version.major, (int) al->http.version.minor);
            out = tmp;
            break;

        case LFT_SERVER_REQ_METHOD:
            if (al->adapted_request) {
                out = al->adapted_request->method.image();
                quote = 1;
            }
            break;

        case LFT_SERVER_REQ_URI:
            // adapted request URI sent to server/peer
            if (al->adapted_request) {
                out = urlCanonical(al->adapted_request);
                quote = 1;
            }
            break;

        case LFT_SERVER_REQ_URLPATH:
            if (al->adapted_request) {
                out = al->adapted_request->urlpath.termedBuf();
                quote = 1;
            }
            break;

        case LFT_SERVER_REQ_VERSION:
            if (al->adapted_request) {
                snprintf(tmp, sizeof(tmp), "%d.%d",
                         (int) al->adapted_request->http_ver.major,
                         (int) al->adapted_request->http_ver.minor);
                out = tmp;
            }
            break;

        case LFT_REQUEST_SIZE_TOTAL:
            outoff = al->cache.requestSize;
            dooff = 1;
            break;

            /*case LFT_REQUEST_SIZE_LINE: */
        case LFT_REQUEST_SIZE_HEADERS:
            outoff = al->cache.requestHeadersSize;
            dooff =1;
            break;
            /*case LFT_REQUEST_SIZE_BODY: */
            /*case LFT_REQUEST_SIZE_BODY_NO_TE: */

        case LFT_REPLY_SIZE_TOTAL:
            outoff = al->cache.replySize;
            dooff = 1;
            break;

        case LFT_REPLY_HIGHOFFSET:
            outoff = al->cache.highOffset;

            dooff = 1;

            break;

        case LFT_REPLY_OBJECTSIZE:
            outoff = al->cache.objectSize;

            dooff = 1;

            break;

            /*case LFT_REPLY_SIZE_LINE: */
        case LFT_REPLY_SIZE_HEADERS:
            outint = al->cache.replyHeadersSize;
            doint = 1;
            break;
            /*case LFT_REPLY_SIZE_BODY: */
            /*case LFT_REPLY_SIZE_BODY_NO_TE: */

        case LFT_TAG:
            if (al->request)
                out = al->request->tag.termedBuf();

            quote = 1;

            break;

        case LFT_IO_SIZE_TOTAL:
            outint = al->cache.requestSize + al->cache.replySize;
            doint = 1;
            break;

        case LFT_EXT_LOG:
            if (al->request)
                out = al->request->extacl_log.termedBuf();

            quote = 1;

            break;

        case LFT_SEQUENCE_NUMBER:
            outoff = logSequenceNumber;
            dooff = 1;
            break;

#if USE_SSL
        case LFT_SSL_BUMP_MODE: {
            const Ssl::BumpMode mode = static_cast<Ssl::BumpMode>(al->ssl.bumpMode);
            // for Ssl::bumpEnd, Ssl::bumpMode() returns NULL and we log '-'
            out = Ssl::bumpMode(mode);
            break;
        }

        case LFT_SSL_USER_CERT_SUBJECT:
            if (X509 *cert = al->cache.sslClientCert.get()) {
                if (X509_NAME *subject = X509_get_subject_name(cert)) {
                    X509_NAME_oneline(subject, tmp, sizeof(tmp));
                    out = tmp;
                }
            }
            break;

        case LFT_SSL_USER_CERT_ISSUER:
            if (X509 *cert = al->cache.sslClientCert.get()) {
                if (X509_NAME *issuer = X509_get_issuer_name(cert)) {
                    X509_NAME_oneline(issuer, tmp, sizeof(tmp));
                    out = tmp;
                }
            }
            break;
#endif

        case LFT_PERCENT:
            out = "%";

            break;
        }

        if (dooff) {
            snprintf(tmp, sizeof(tmp), "%0*" PRId64, fmt->zero && fmt->widthMin >= 0 ? fmt->widthMin : 0, outoff);
            out = tmp;

        } else if (doint) {
            snprintf(tmp, sizeof(tmp), "%0*ld", fmt->zero && fmt->widthMin >= 0 ? fmt->widthMin : 0, outint);
            out = tmp;
        }

        if (out && *out) {
            if (quote || fmt->quote != LOG_QUOTE_NONE) {
                char *newout = NULL;
                int newfree = 0;

                switch (fmt->quote) {

                case LOG_QUOTE_NONE:
                    newout = rfc1738_escape_unescaped(out);
                    break;

                case LOG_QUOTE_QUOTES: {
                    size_t out_len = static_cast<size_t>(strlen(out)) * 2 + 1;
                    if (out_len >= sizeof(tmp)) {
                        newout = (char *)xmalloc(out_len);
                        newfree = 1;
                    } else
                        newout = tmp;
                    log_quoted_string(out, newout);
                }
                break;

                case LOG_QUOTE_MIMEBLOB:
                    newout = QuoteMimeBlob(out);
                    newfree = 1;
                    break;

                case LOG_QUOTE_URL:
                    newout = rfc1738_escape(out);
                    break;

                case LOG_QUOTE_RAW:
                    break;
                }

                if (newout) {
                    if (dofree)
                        safe_free(out);

                    out = newout;

                    dofree = newfree;
                }
            }

            // enforce width limits if configured
            const bool haveMaxWidth = fmt->widthMax >=0 && !doint && !dooff;
            if (haveMaxWidth || fmt->widthMin) {
                const int minWidth = fmt->widthMin >= 0 ?
                                     fmt->widthMin :0;
                const int maxWidth = haveMaxWidth ?
                                     fmt->widthMax : strlen(out);

                if (fmt->left)
                    mb.Printf("%-*.*s", minWidth, maxWidth, out);
                else
                    mb.Printf("%*.*s", minWidth, maxWidth, out);
            } else
                mb.append(out, strlen(out));
        } else {
            mb.append("-", 1);
        }

        if (fmt->space)
            mb.append(" ", 1);

        sb.clean();

        if (dofree)
            safe_free(out);
    }
}