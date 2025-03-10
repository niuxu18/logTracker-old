static apr_status_t parse_chunk_size(http_ctx_t *ctx, const char *buffer,
                                     apr_size_t len, int linelimit, int strict)
{
    apr_size_t i = 0;

    while (i < len) {
        char c = buffer[i];

        ap_xlate_proto_from_ascii(&c, 1);

        /* handle CRLF after the chunk */
        if (ctx->state == BODY_CHUNK_END
                || ctx->state == BODY_CHUNK_END_LF) {
            if (c == LF) {
                if (strict && (ctx->state != BODY_CHUNK_END_LF)) {
                    /*
                     * CR missing before LF.
                     */
                    return APR_EINVAL;
                }
                ctx->state = BODY_CHUNK;
            }
            else if (c == CR && ctx->state == BODY_CHUNK_END) {
                ctx->state = BODY_CHUNK_END_LF;
            }
            else {
                /*
                 * CRLF expected.
                 */
                return APR_EINVAL;
            }
            i++;
            continue;
        }

        /* handle start of the chunk */
        if (ctx->state == BODY_CHUNK) {
            if (!apr_isxdigit(c)) {
                /*
                 * Detect invalid character at beginning. This also works for
                 * empty chunk size lines.
                 */
                return APR_EINVAL;
            }
            else {
                ctx->state = BODY_CHUNK_PART;
            }
            ctx->remaining = 0;
            ctx->chunkbits = sizeof(apr_off_t) * 8;
            ctx->chunk_used = 0;
            ctx->chunk_bws = 0;
        }

        if (c == LF) {
            if (strict && (ctx->state != BODY_CHUNK_LF)) {
                /*
                 * CR missing before LF.
                 */
                return APR_EINVAL;
            }
            if (ctx->remaining) {
                ctx->state = BODY_CHUNK_DATA;
            }
            else {
                ctx->state = BODY_CHUNK_TRAILER;
            }
        }
        else if (ctx->state == BODY_CHUNK_LF) {
            /*
             * LF expected.
             */
            return APR_EINVAL;
        }
        else if (c == CR) {
            ctx->state = BODY_CHUNK_LF;
        }
        else if (c == ';') {
            ctx->state = BODY_CHUNK_EXT;
        }
        else if (ctx->state == BODY_CHUNK_EXT) {
            /*
             * Control chars (excluding tabs) are invalid.
             * TODO: more precisely limit input
             */
            if (c != '\t' && apr_iscntrl(c)) {
                return APR_EINVAL;
            }
        }
        else if (c == ' ' || c == '\t') {
            /* Be lenient up to 10 implied *LWS, a legacy of RFC 2616,
             * and noted as errata to RFC7230;
             * https://www.rfc-editor.org/errata_search.php?rfc=7230&eid=4667
             */
            ctx->state = BODY_CHUNK_CR;
            if (++ctx->chunk_bws > 10) {
                return APR_EINVAL;
            }
        }
        else if (ctx->state == BODY_CHUNK_CR) {
            /*
             * ';', CR or LF expected.
             */
            return APR_EINVAL;
        }
        else if (ctx->state == BODY_CHUNK_PART) {
            int xvalue;

            /* ignore leading zeros */
            if (!ctx->remaining && c == '0') {
                i++;
                continue;
            }

            ctx->chunkbits -= 4;
            if (ctx->chunkbits < 0) {
                /* overflow */
                return APR_ENOSPC;
            }

            if (c >= '0' && c <= '9') {
                xvalue = c - '0';
            }
            else if (c >= 'A' && c <= 'F') {
                xvalue = c - 'A' + 0xa;
            }
            else if (c >= 'a' && c <= 'f') {
                xvalue = c - 'a' + 0xa;
            }
            else {
                /* bogus character */
                return APR_EINVAL;
            }

            ctx->remaining = (ctx->remaining << 4) | xvalue;
            if (ctx->remaining < 0) {
                /* overflow */
                return APR_ENOSPC;
            }
        }
        else {
            /* Should not happen */
            return APR_EGENERAL;
        }

        i++;
    }

    /* sanity check */
    ctx->chunk_used += len;
    if (ctx->chunk_used < 0 || ctx->chunk_used > linelimit) {
        return APR_ENOSPC;
    }

    return APR_SUCCESS;
}