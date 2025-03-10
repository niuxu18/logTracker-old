static apr_ssize_t send_response_header(isapi_cid *cid, 
                                        const char *stat,
                                        const char *head, 
                                        apr_size_t statlen,
                                        apr_size_t headlen)
{
    int head_present = 1;
    int termarg;
    char *termch;
    apr_size_t ate = 0;

    if (!head || headlen == 0 || !*head) {
        head = stat;
        stat = NULL;
        headlen = statlen;
        statlen = 0;
        head_present = 0; /* Don't eat the header */
    }

    if (!stat || statlen == 0 || !*stat) {
        if (head && headlen && *head && ((stat = memchr(head, '\r', headlen))
                                      || (stat = memchr(head, '\n', headlen))
                                      || (stat = memchr(head, '\0', headlen))
                                      || (stat = head + headlen))) {
            statlen = stat - head;
            if (memchr(head, ':', statlen)) {
                stat = "Status: 200 OK";
                statlen = strlen(stat);
            }
            else {
                const char *flip = head;
                head = stat;
                stat = flip;
                headlen -= statlen;
                ate += statlen;
                if (*head == '\r' && headlen)
                    ++head, --headlen, ++ate;
                if (*head == '\n' && headlen)
                    ++head, --headlen, ++ate;
            }
        }
    }

    if (stat && (statlen > 0) && *stat) {
        char *newstat;
        if (!apr_isdigit(*stat)) {
            const char *stattok = stat;
            int toklen = statlen;
            while (toklen && *stattok && !apr_isspace(*stattok)) {
                ++stattok; --toklen;
            }
            while (toklen && apr_isspace(*stattok)) {
                ++stattok; --toklen;
            }
            /* Now decide if we follow the xxx message 
             * or the http/x.x xxx message format 
             */
            if (toklen && apr_isdigit(*stattok)) {
                statlen -= toklen;
                stat = stattok;
            }
        }
        newstat = apr_palloc(cid->r->pool, statlen + 9);
        strcpy(newstat, "Status: ");
        apr_cpystrn(newstat + 8, stat, statlen + 1);
        stat = newstat;
        statlen += 8;
    }

    if (!head || headlen == 0 || !*head) {
        head = "\r\n";
        headlen = 2;
    }
    else
    {
        if (head[headlen - 1] && head[headlen]) {
            /* Whoops... not NULL terminated */
            head = apr_pstrndup(cid->r->pool, head, headlen);
        }
    }
 
    /* Seems IIS does not enforce the requirement for \r\n termination 
     * on HSE_REQ_SEND_RESPONSE_HEADER, but we won't panic... 
     * ap_scan_script_header_err_strs handles this aspect for us.
     *
     * Parse them out, or die trying 
     */
    if (stat) {
        cid->r->status = ap_scan_script_header_err_strs(cid->r, NULL, 
                                        &termch, &termarg, stat, head, NULL);
        cid->ecb->dwHttpStatusCode = cid->r->status;
    }
    else {
        cid->r->status = ap_scan_script_header_err_strs(cid->r, NULL, 
                                        &termch, &termarg, head, NULL);
        if (cid->ecb->dwHttpStatusCode && cid->r->status == HTTP_OK
                && cid->ecb->dwHttpStatusCode != HTTP_OK) {
            /* We tried every way to Sunday to get the status...
             * so now we fall back on dwHttpStatusCode if it appears
             * ap_scan_script_header fell back on the default code.
             * Any other results set dwHttpStatusCode to the decoded 
             * status value.
             */
            cid->r->status = cid->ecb->dwHttpStatusCode;
            cid->r->status_line = ap_get_status_line(cid->r->status);
        }
        else {
            cid->ecb->dwHttpStatusCode = cid->r->status;
        }
    }
    if (cid->r->status == HTTP_INTERNAL_SERVER_ERROR) {
        return -1;
    }

    /* If only Status was passed, we consumed nothing 
     */
    if (!head_present)
        return 0;

    cid->headers_set = 1;

    /* If all went well, tell the caller we consumed the headers complete 
     */
    if (!termch)
        return(ate + headlen);

    /* Any data left must be sent directly by the caller, all we
     * give back is the size of the headers we consumed (which only
     * happens if the parser got to the head arg, which varies based
     * on whether we passed stat+head to scan, or only head.
     */
    if (termch && (termarg == (stat ? 1 : 0))
               && head_present && head + headlen > termch) {
        return ate + termch - head;
    }
    return ate;
}