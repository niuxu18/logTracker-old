    }
    return 0;
}

/* -------------------------- The main function --------------------------- */

/*
 * returns the index position of the first byte of start_seq (or the len of
 * the buffer as non-match)
 */
static apr_size_t find_start_sequence(ssi_ctx_t *ctx, const char *data,
                                      apr_size_t len)
{
    apr_size_t slen = ctx->ctx->start_seq_len;
    apr_size_t index;
    const char *p, *ep;

    if (len < slen) {
        p = data; /* try partial match at the end of the buffer (below) */
    }
    else {
        /* try fast bndm search over the buffer
         * (hopefully the whole start sequence can be found in this buffer)
         */
        index = bndm(ctx->ctx->start_seq, ctx->ctx->start_seq_len, data, len,
                     ctx->ctx->start_seq_pat);

        /* wow, found it. ready. */
        if (index < len) {
            ctx->state = PARSE_DIRECTIVE;
            return index;
        }
        else {
            /* ok, the pattern can't be found as whole in the buffer,
             * check the end for a partial match
             */
            p = data + len - slen + 1;
        }
    }

    ep = data + len;
    do {
        while (p < ep && *p != *ctx->ctx->start_seq) {
            ++p;
        }

        index = p - data;

        /* found a possible start_seq start */
        if (p < ep) {
            apr_size_t pos = 1;

            ++p;
            while (p < ep && *p == ctx->ctx->start_seq[pos]) {
                ++p;
                ++pos;
            }

            /* partial match found. Store the info for the next round */
            if (p == ep) {
                ctx->state = PARSE_HEAD;
                ctx->ctx->parse_pos = pos;
                return index;
            }
        }

        /* we must try all combinations; consider (e.g.) SSIStartTag "--->"
         * and a string data of "--.-" and the end of the buffer
         */
        p = data + index + 1;
    } while (p < ep);

    /* no match */
    return len;
}

/*
 * returns the first byte *after* the partial (or final) match.
 *
 * If we had to trick with the start_seq start, 'release' returns the
 * number of chars of the start_seq which appeared not to be part of a
 * full tag and may have to be passed down the filter chain.
 */
static apr_size_t find_partial_start_sequence(ssi_ctx_t *ctx,
                                              const char *data,
                                              apr_size_t len,
                                              apr_size_t *release)
{
    apr_size_t pos, spos = 0;
    apr_size_t slen = ctx->ctx->start_seq_len;
    const char *p, *ep;

    pos = ctx->ctx->parse_pos;
    ep = data + len;
    *release = 0;

    do {
        p = data;

        while (p < ep && pos < slen && *p == ctx->ctx->start_seq[pos]) {
            ++p;
            ++pos;
        }

        /* full match */
        if (pos == slen) {
            ctx->state = PARSE_DIRECTIVE;
            return (p - data);
        }

        /* the whole buffer is a partial match */
        if (p == ep) {
            ctx->ctx->parse_pos = pos;
            return (p - data);
        }

        /* No match so far, but again:
         * We must try all combinations, since the start_seq is a random
         * user supplied string
         *
         * So: look if the first char of start_seq appears somewhere within
         * the current partial match. If it does, try to start a match that
         * begins with this offset. (This can happen, if a strange
         * start_seq like "---->" spans buffers)
         */
        if (spos < ctx->ctx->parse_pos) {
            do {
                ++spos;
                ++*release;
                p = ctx->ctx->start_seq + spos;
                pos = ctx->ctx->parse_pos - spos;

                while (pos && *p != *ctx->ctx->start_seq) {
                    ++p;
                    ++spos;
                    ++*release;
                    --pos;
                }

                /* if a matching beginning char was found, try to match the
                 * remainder of the old buffer.
                 */
                if (pos > 1) {
                    apr_size_t t = 1;

                    ++p;
                    while (t < pos && *p == ctx->ctx->start_seq[t]) {
                        ++p;
                        ++t;
                    }

                    if (t == pos) {
                        /* yeah, another partial match found in the *old*
                         * buffer, now test the *current* buffer for
                         * continuing match
                         */
                        break;
                    }
                }
            } while (pos > 1);

            if (pos) {
                continue;
            }
        }

        break;
    } while (1); /* work hard to find a match ;-) */

    /* no match at all, release all (wrongly) matched chars so far */
    *release = ctx->ctx->parse_pos;
    ctx->state = PARSE_PRE_HEAD;
    return 0;
}

/*
 * returns the position after the directive
 */
static apr_size_t find_directive(ssi_ctx_t *ctx, const char *data,
                                 apr_size_t len, char ***store,
                                 apr_size_t **store_len)
{
    const char *p = data;
    const char *ep = data + len;
    apr_size_t pos;

    switch (ctx->state) {
    case PARSE_DIRECTIVE:
        while (p < ep && !apr_isspace(*p)) {
            /* we have to consider the case of missing space between directive
             * and end_seq (be somewhat lenient), e.g. <!--#printenv-->
             */
            if (*p == *ctx->ctx->end_seq) {
                ctx->state = PARSE_DIRECTIVE_TAIL;
                ctx->ctx->parse_pos = 1;
                ++p;
                return (p - data);
            }
            ++p;
        }

        if (p < ep) { /* found delimiter whitespace */
            ctx->state = PARSE_DIRECTIVE_POSTNAME;
            *store = &ctx->directive;
            *store_len = &ctx->ctx->directive_length;
        }

        break;

    case PARSE_DIRECTIVE_TAIL:
        pos = ctx->ctx->parse_pos;

        while (p < ep && pos < ctx->end_seq_len &&
               *p == ctx->ctx->end_seq[pos]) {
            ++p;
            ++pos;
        }

        /* full match, we're done */
        if (pos == ctx->end_seq_len) {
            ctx->state = PARSE_DIRECTIVE_POSTTAIL;
            *store = &ctx->directive;
            *store_len = &ctx->ctx->directive_length;
            break;
        }

        /* partial match, the buffer is too small to match fully */
        if (p == ep) {
            ctx->ctx->parse_pos = pos;
            break;
        }

        /* no match. continue normal parsing */
        ctx->state = PARSE_DIRECTIVE;
        return 0;

    case PARSE_DIRECTIVE_POSTTAIL:
        ctx->state = PARSE_EXECUTE;
        ctx->ctx->directive_length -= ctx->end_seq_len;
        /* continue immediately with the next state */

    case PARSE_DIRECTIVE_POSTNAME:
        if (PARSE_DIRECTIVE_POSTNAME == ctx->state) {
            ctx->state = PARSE_PRE_ARG;
        }
        ctx->argc = 0;
        ctx->argv = NULL;

        if (!ctx->ctx->directive_length) {
            ctx->error = 1;
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, ctx->r, "missing directive "
                          "name in parsed document %s", ctx->r->filename);
        }
        else {
            char *sp = ctx->directive;
            char *sep = ctx->directive + ctx->ctx->directive_length;

            /* normalize directive name */
            for (; sp < sep; ++sp) {
                *sp = apr_tolower(*sp);
            }
        }

        return 0;

    default:
        /* get a rid of a gcc warning about unhandled enumerations */
        break;
    }

    return (p - data);
}

/*
 * find out whether the next token is (a possible) end_seq or an argument
 */
static apr_size_t find_arg_or_tail(ssi_ctx_t *ctx, const char *data,
                                   apr_size_t len)
{
    const char *p = data;
    const char *ep = data + len;

    /* skip leading WS */
    while (p < ep && apr_isspace(*p)) {
        ++p;
    }

    /* buffer doesn't consist of whitespaces only */
    if (p < ep) {
        ctx->state = (*p == *ctx->ctx->end_seq) ? PARSE_TAIL : PARSE_ARG;
    }

    return (p - data);
}

/*
 * test the stream for end_seq. If it doesn't match at all, it must be an
 * argument
 */
static apr_size_t find_tail(ssi_ctx_t *ctx, const char *data,
                            apr_size_t len)
{
    const char *p = data;
    const char *ep = data + len;
    apr_size_t pos = ctx->ctx->parse_pos;

    if (PARSE_TAIL == ctx->state) {
        ctx->state = PARSE_TAIL_SEQ;
        pos = ctx->ctx->parse_pos = 0;
    }

    while (p < ep && pos < ctx->end_seq_len && *p == ctx->ctx->end_seq[pos]) {
        ++p;
        ++pos;
    }

    /* bingo, full match */
    if (pos == ctx->end_seq_len) {
        ctx->state = PARSE_EXECUTE;
        return (p - data);
    }

    /* partial match, the buffer is too small to match fully */
    if (p == ep) {
        ctx->ctx->parse_pos = pos;
        return (p - data);
    }

    /* no match. It must be an argument string then */
    ctx->state = PARSE_ARG;
    return 0;
}

/*
 * extract name=value from the buffer
 * A pcre-pattern could look (similar to):
 * name\s*(?:=\s*(["'`]?)value\1(?>\s*))?
 */
static apr_size_t find_argument(ssi_ctx_t *ctx, const char *data,
                                apr_size_t len, char ***store,
                                apr_size_t **store_len)
{
    const char *p = data;
    const char *ep = data + len;

    switch (ctx->state) {
    case PARSE_ARG:
        /*
         * create argument structure and append it to the current list
         */
        ctx->current_arg = apr_palloc(ctx->dpool,
                                      sizeof(*ctx->current_arg));
        ctx->current_arg->next = NULL;

        ++(ctx->argc);
        if (!ctx->argv) {
            ctx->argv = ctx->current_arg;
        }
        else {
            ssi_arg_item_t *newarg = ctx->argv;

            while (newarg->next) {
                newarg = newarg->next;
            }
            newarg->next = ctx->current_arg;
        }

        /* check whether it's a valid one. If it begins with a quote, we
         * can safely assume, someone forgot the name of the argument
         */
        switch (*p) {
        case '"': case '\'': case '`':
            *store = NULL;

            ctx->state = PARSE_ARG_VAL;
            ctx->quote = *p++;
            ctx->current_arg->name = NULL;
            ctx->current_arg->name_len = 0;
            ctx->error = 1;

            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, ctx->r, "missing argument "
                          "name for value to tag %s in %s",
                          apr_pstrmemdup(ctx->r->pool, ctx->directive,
                                         ctx->ctx->directive_length),
                                         ctx->r->filename);

            return (p - data);

        default:
            ctx->state = PARSE_ARG_NAME;
        }
        /* continue immediately with next state */

    case PARSE_ARG_NAME:
        while (p < ep && !apr_isspace(*p) && *p != '=') {
            ++p;
        }

        if (p < ep) {
            ctx->state = PARSE_ARG_POSTNAME;
            *store = &ctx->current_arg->name;
            *store_len = &ctx->current_arg->name_len;
            return (p - data);
        }
        break;

    case PARSE_ARG_POSTNAME:
        ctx->current_arg->name = apr_pstrmemdup(ctx->dpool,
                                                ctx->current_arg->name,
                                                ctx->current_arg->name_len);
        if (!ctx->current_arg->name_len) {
            ctx->error = 1;
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, ctx->r, "missing argument "
                          "name for value to tag %s in %s",
                          apr_pstrmemdup(ctx->r->pool, ctx->directive,
                                         ctx->ctx->directive_length),
                                         ctx->r->filename);
        }
        else {
            char *sp = ctx->current_arg->name;

            /* normalize the name */
            while (*sp) {
                *sp = apr_tolower(*sp);
                ++sp;
            }
        }

        ctx->state = PARSE_ARG_EQ;
        /* continue with next state immediately */

    case PARSE_ARG_EQ:
        *store = NULL;

        while (p < ep && apr_isspace(*p)) {
            ++p;
        }

        if (p < ep) {
            if (*p == '=') {
                ctx->state = PARSE_ARG_PREVAL;
                ++p;
            }
            else { /* no value */
                ctx->current_arg->value = NULL;
                ctx->state = PARSE_PRE_ARG;
            }

            return (p - data);
        }
        break;

    case PARSE_ARG_PREVAL:
        *store = NULL;

        while (p < ep && apr_isspace(*p)) {
            ++p;
        }

        /* buffer doesn't consist of whitespaces only */
        if (p < ep) {
            ctx->state = PARSE_ARG_VAL;
            switch (*p) {
            case '"': case '\'': case '`':
                ctx->quote = *p++;
                break;
            default:
                ctx->quote = '\0';
                break;
            }

            return (p - data);
        }
        break;

    case PARSE_ARG_VAL_ESC:
        if (*p == ctx->quote) {
            ++p;
        }
        ctx->state = PARSE_ARG_VAL;
        /* continue with next state immediately */

    case PARSE_ARG_VAL:
        for (; p < ep; ++p) {
            if (ctx->quote && *p == '\\') {
                ++p;
                if (p == ep) {
                    ctx->state = PARSE_ARG_VAL_ESC;
                    break;
                }

                if (*p != ctx->quote) {
                    --p;
                }
            }
            else if (ctx->quote && *p == ctx->quote) {
                ++p;
                *store = &ctx->current_arg->value;
                *store_len = &ctx->current_arg->value_len;
                ctx->state = PARSE_ARG_POSTVAL;
                break;
            }
            else if (!ctx->quote && apr_isspace(*p)) {
                ++p;
                *store = &ctx->current_arg->value;
                *store_len = &ctx->current_arg->value_len;
                ctx->state = PARSE_ARG_POSTVAL;
                break;
            }
        }

        return (p - data);

    case PARSE_ARG_POSTVAL:
        /*
         * The value is still the raw input string. Finally clean it up.
         */
        --(ctx->current_arg->value_len);

        /* strip quote escaping \ from the string */
        if (ctx->quote) {
            apr_size_t shift = 0;
            char *sp;

            sp = ctx->current_arg->value;
            ep = ctx->current_arg->value + ctx->current_arg->value_len;
            while (sp < ep && *sp != '\\') {
                ++sp;
            }
            for (; sp < ep; ++sp) {
                if (*sp == '\\' && sp[1] == ctx->quote) {
                    ++sp;
                    ++shift;
                }
                if (shift) {
                    *(sp-shift) = *sp;
                }
            }

            ctx->current_arg->value_len -= shift;
        }

        ctx->current_arg->value[ctx->current_arg->value_len] = '\0';
        ctx->state = PARSE_PRE_ARG;

        return 0;

    default:
        /* get a rid of a gcc warning about unhandled enumerations */
        break;
    }

    return len; /* partial match of something */
}

/*
 * This is the main loop over the current bucket brigade.
 */
static apr_status_t send_parsed_content(ap_filter_t *f, apr_bucket_brigade *bb)
{
    ssi_ctx_t *ctx = f->ctx;
    request_rec *r = f->r;
    apr_bucket *b = APR_BRIGADE_FIRST(bb);
    apr_bucket_brigade *pass_bb;
    apr_status_t rv = APR_SUCCESS;
    char *magic; /* magic pointer for sentinel use */

    /* fast exit */
    if (APR_BRIGADE_EMPTY(bb)) {
        return APR_SUCCESS;
    }

    /* we may crash, since already cleaned up; hand over the responsibility
     * to the next filter;-)
     */
    if (ctx->seen_eos) {
        return ap_pass_brigade(f->next, bb);
    }

    /* All stuff passed along has to be put into that brigade */
    pass_bb = apr_brigade_create(ctx->ctx->pool, f->c->bucket_alloc);
    ctx->ctx->bytes_parsed = 0;
    ctx->ctx->output_now = 0;
    ctx->error = 0;

    /* loop over the current bucket brigade */
    while (b != APR_BRIGADE_SENTINEL(bb)) {
        const char *data = NULL;
        apr_size_t len, index, release;
        apr_bucket *newb = NULL;
        char **store = &magic;
        apr_size_t *store_len;

        /* handle meta buckets before reading any data */
        if (APR_BUCKET_IS_METADATA(b)) {
            newb = APR_BUCKET_NEXT(b);

            APR_BUCKET_REMOVE(b);

            if (APR_BUCKET_IS_EOS(b)) {
                ctx->seen_eos = 1;

                /* Hit end of stream, time for cleanup ... But wait!
                 * Perhaps we're not ready yet. We may have to loop one or
                 * two times again to finish our work. In that case, we
                 * just re-insert the EOS bucket to allow for an extra loop.
                 *
                 * PARSE_EXECUTE means, we've hit a directive just before the
                 *    EOS, which is now waiting for execution.
                 *
                 * PARSE_DIRECTIVE_POSTTAIL means, we've hit a directive with
                 *    no argument and no space between directive and end_seq
                 *    just before the EOS. (consider <!--#printenv--> as last
                 *    or only string within the stream). This state, however,
                 *    just cleans up and turns itself to PARSE_EXECUTE, which
                 *    will be passed through within the next (and actually
                 *    last) round.
                 */
                if (PARSE_EXECUTE            == ctx->state ||
                    PARSE_DIRECTIVE_POSTTAIL == ctx->state) {
                    APR_BUCKET_INSERT_BEFORE(newb, b);
                }
                else {
                    break; /* END OF STREAM */
                }
            }
            else {
                APR_BRIGADE_INSERT_TAIL(pass_bb, b);

                if (APR_BUCKET_IS_FLUSH(b)) {
                    ctx->ctx->output_now = 1;
                }

                b = newb;
                continue;
            }
        }

        /* enough is enough ... */
        if (ctx->ctx->output_now ||
            ctx->ctx->bytes_parsed > AP_MIN_BYTES_TO_WRITE) {

            if (!APR_BRIGADE_EMPTY(pass_bb)) {
                rv = ap_pass_brigade(f->next, pass_bb);
                if (!APR_STATUS_IS_SUCCESS(rv)) {
                    apr_brigade_destroy(pass_bb);
                    return rv;
                }
            }

            ctx->ctx->output_now = 0;
            ctx->ctx->bytes_parsed = 0;
        }

        /* read the current bucket data */
        len = 0;
        if (!ctx->seen_eos) {
            if (ctx->ctx->bytes_parsed > 0) {
                rv = apr_bucket_read(b, &data, &len, APR_NONBLOCK_READ);
                if (APR_STATUS_IS_EAGAIN(rv)) {
                    ctx->ctx->output_now = 1;
                    continue;
                }
            }

            if (!len || !APR_STATUS_IS_SUCCESS(rv)) {
                rv = apr_bucket_read(b, &data, &len, APR_BLOCK_READ);
            }

            if (!APR_STATUS_IS_SUCCESS(rv)) {
                apr_brigade_destroy(pass_bb);
                return rv;
            }

            ctx->ctx->bytes_parsed += len;
        }

        /* zero length bucket, fetch next one */
        if (!len && !ctx->seen_eos) {
            b = APR_BUCKET_NEXT(b);
            continue;
        }

        /*
         * it's actually a data containing bucket, start/continue parsing
         */

        switch (ctx->state) {
        /* no current tag; search for start sequence */
        case PARSE_PRE_HEAD:
            index = find_start_sequence(ctx, data, len);

            if (index < len) {
                apr_bucket_split(b, index);
            }

            newb = APR_BUCKET_NEXT(b);
            if (ctx->ctx->flags & FLAG_PRINTING) {
                APR_BUCKET_REMOVE(b);
                APR_BRIGADE_INSERT_TAIL(pass_bb, b);
            }
            else {
                apr_bucket_delete(b);
            }

            if (index < len) {
                /* now delete the start_seq stuff from the remaining bucket */
                if (PARSE_DIRECTIVE == ctx->state) { /* full match */
                    apr_bucket_split(newb, ctx->ctx->start_seq_len);
                    ctx->ctx->output_now = 1; /* pass pre-tag stuff */
                }

                b = APR_BUCKET_NEXT(newb);
                apr_bucket_delete(newb);
            }
            else {
                b = newb;
            }

            break;

        /* we're currently looking for the end of the start sequence */
        case PARSE_HEAD:
            index = find_partial_start_sequence(ctx, data, len, &release);

            /* check if we mismatched earlier and have to release some chars */
            if (release && (ctx->ctx->flags & FLAG_PRINTING)) {
                char *to_release = apr_palloc(ctx->ctx->pool, release);

                memcpy(to_release, ctx->ctx->start_seq, release);
                newb = apr_bucket_pool_create(to_release, release,
                                              ctx->ctx->pool,
                                              f->c->bucket_alloc);
                APR_BRIGADE_INSERT_TAIL(pass_bb, newb);
            }

            if (index) { /* any match */
                /* now delete the start_seq stuff from the remaining bucket */
                if (PARSE_DIRECTIVE == ctx->state) { /* final match */
                    apr_bucket_split(b, index);
                    ctx->ctx->output_now = 1; /* pass pre-tag stuff */
                }
                newb = APR_BUCKET_NEXT(b);
                apr_bucket_delete(b);
                b = newb;
            }

            break;

        /* we're currently grabbing the directive name */
        case PARSE_DIRECTIVE:
        case PARSE_DIRECTIVE_POSTNAME:
        case PARSE_DIRECTIVE_TAIL:
        case PARSE_DIRECTIVE_POSTTAIL:
            index = find_directive(ctx, data, len, &store, &store_len);

            if (index) {
                apr_bucket_split(b, index);
                newb = APR_BUCKET_NEXT(b);
            }

            if (store) {
                if (index) {
                    APR_BUCKET_REMOVE(b);
                    APR_BRIGADE_INSERT_TAIL(ctx->tmp_bb, b);
                    b = newb;
                }

                /* time for cleanup? */
                if (store != &magic) {
                    apr_brigade_pflatten(ctx->tmp_bb, store, store_len,
                                         ctx->dpool);
                    apr_brigade_cleanup(ctx->tmp_bb);
                }
            }
            else if (index) {
                apr_bucket_delete(b);
                b = newb;
            }

            break;

        /* skip WS and find out what comes next (arg or end_seq) */
        case PARSE_PRE_ARG:
            index = find_arg_or_tail(ctx, data, len);

            if (index) { /* skipped whitespaces */
                if (index < len) {
                    apr_bucket_split(b, index);
                }
                newb = APR_BUCKET_NEXT(b);
                apr_bucket_delete(b);
                b = newb;
            }

            break;

        /* currently parsing name[=val] */
        case PARSE_ARG:
        case PARSE_ARG_NAME:
        case PARSE_ARG_POSTNAME:
        case PARSE_ARG_EQ:
        case PARSE_ARG_PREVAL:
        case PARSE_ARG_VAL:
        case PARSE_ARG_VAL_ESC:
        case PARSE_ARG_POSTVAL:
            index = find_argument(ctx, data, len, &store, &store_len);

            if (index) {
                apr_bucket_split(b, index);
                newb = APR_BUCKET_NEXT(b);
            }

            if (store) {
                if (index) {
                    APR_BUCKET_REMOVE(b);
                    APR_BRIGADE_INSERT_TAIL(ctx->tmp_bb, b);
                    b = newb;
                }

                /* time for cleanup? */
                if (store != &magic) {
                    apr_brigade_pflatten(ctx->tmp_bb, store, store_len,
                                         ctx->dpool);
                    apr_brigade_cleanup(ctx->tmp_bb);
                }
            }
            else if (index) {
                apr_bucket_delete(b);
                b = newb;
            }

            break;

        /* try to match end_seq at current pos. */
        case PARSE_TAIL:
        case PARSE_TAIL_SEQ:
            index = find_tail(ctx, data, len);

            switch (ctx->state) {
            case PARSE_EXECUTE:  /* full match */
                apr_bucket_split(b, index);
                newb = APR_BUCKET_NEXT(b);
                apr_bucket_delete(b);
                b = newb;
                break;

            case PARSE_ARG:      /* no match */
                /* PARSE_ARG must reparse at the beginning */
                APR_BRIGADE_PREPEND(bb, ctx->tmp_bb);
                b = APR_BRIGADE_FIRST(bb);
                break;

            default:             /* partial match */
                newb = APR_BUCKET_NEXT(b);
                APR_BUCKET_REMOVE(b);
                APR_BRIGADE_INSERT_TAIL(ctx->tmp_bb, b);
                b = newb;
                break;
            }

            break;

        /* now execute the parsed directive, cleanup the space and
         * start again with PARSE_PRE_HEAD
         */
        case PARSE_EXECUTE:
            /* if there was an error, it was already logged; just stop here */
            if (ctx->error) {
                if (ctx->ctx->flags & FLAG_PRINTING) {
                    SSI_CREATE_ERROR_BUCKET(ctx->ctx, f, pass_bb);
                    ctx->error = 0;
                }
            }
            else {
                include_handler_fn_t *handle_func;

                handle_func =
                    (include_handler_fn_t *) apr_hash_get(include_hash,
                                                    ctx->directive,
                                                    ctx->ctx->directive_length);
                if (handle_func) {
                    apr_bucket *dummy;
                    char *tag;
                    apr_size_t tag_len = 0;
                    ssi_arg_item_t *carg = ctx->argv;

                    /* legacy wrapper code */
                    while (carg) {
                        /* +1 \0 byte (either after tag or value)
                         * +1 =  byte (before value)
                         */
                        tag_len += (carg->name  ? carg->name_len      : 0) +
                                   (carg->value ? carg->value_len + 1 : 0) + 1;
                        carg = carg->next;
                    }

                    tag = ctx->ctx->combined_tag = ctx->ctx->curr_tag_pos =
                        apr_palloc(ctx->dpool, tag_len);

                    carg = ctx->argv;
                    while (carg) {
                        if (carg->name) {
                            memcpy(tag, carg->name, carg->name_len);
                            tag += carg->name_len;
                        }
                        if (carg->value) {
                            *tag++ = '=';
                            memcpy(tag, carg->value, carg->value_len);
                            tag += carg->value_len;
                        }
                        *tag++ = '\0';
                        carg = carg->next;
                    }
                    ctx->ctx->tag_length = tag_len;

                    /* create dummy buckets for backards compat */
                    ctx->ctx->head_start_bucket =
                        apr_bucket_pool_create(apr_pmemdup(ctx->ctx->pool,
                                                           ctx->ctx->start_seq,
                                                       ctx->ctx->start_seq_len),
                                               ctx->ctx->start_seq_len,
                                               ctx->ctx->pool,
                                               f->c->bucket_alloc);
                    APR_BRIGADE_INSERT_TAIL(ctx->ctx->ssi_tag_brigade,
                                            ctx->ctx->head_start_bucket);
                    ctx->ctx->tag_start_bucket =
                        apr_bucket_pool_create(apr_pmemdup(ctx->ctx->pool,
                                                         ctx->ctx->combined_tag,
                                                         ctx->ctx->tag_length),
                                               ctx->ctx->tag_length,
                                               ctx->ctx->pool,
                                               f->c->bucket_alloc);
                    APR_BRIGADE_INSERT_TAIL(ctx->ctx->ssi_tag_brigade,
                                            ctx->ctx->tag_start_bucket);
                    ctx->ctx->tail_start_bucket =
                        apr_bucket_pool_create(apr_pmemdup(ctx->ctx->pool,
                                                           ctx->ctx->end_seq,
                                                           ctx->end_seq_len),
                                               ctx->end_seq_len,
                                               ctx->ctx->pool,
                                               f->c->bucket_alloc);
                    APR_BRIGADE_INSERT_TAIL(ctx->ctx->ssi_tag_brigade,
                                            ctx->ctx->tail_start_bucket);

                    rv = handle_func(ctx->ctx, &bb, r, f, b, &dummy);

                    apr_brigade_cleanup(ctx->ctx->ssi_tag_brigade);

                    if (rv != 0 && rv != 1 && rv != -1) {
                        apr_brigade_destroy(pass_bb);
                        return rv;
                    }

                    if (dummy) {
                        apr_bucket_brigade *remain;

                        remain = apr_brigade_split(bb, b);
                        APR_BRIGADE_CONCAT(pass_bb, bb);
                        bb = remain;
                    }
                }
                else {
                    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                                  "unknown directive \"%s\" in parsed doc %s",
                                  apr_pstrmemdup(r->pool, ctx->directive,
                                                 ctx->ctx->directive_length),
                                                 r->filename);
                    if (ctx->ctx->flags & FLAG_PRINTING) {
                        SSI_CREATE_ERROR_BUCKET(ctx->ctx, f, pass_bb);
                    }
                }
            }

            /* cleanup */
            apr_pool_clear(ctx->dpool);
            apr_brigade_cleanup(ctx->tmp_bb);

            /* Oooof. Done here, start next round */
            ctx->state = PARSE_PRE_HEAD;
            break;
        }

    } /* while (brigade) */

    /* End of stream. Final cleanup */
    if (ctx->seen_eos) {
        if (PARSE_HEAD == ctx->state) {
            if (ctx->ctx->flags & FLAG_PRINTING) {
                char *to_release = apr_palloc(ctx->ctx->pool,
                                              ctx->ctx->parse_pos);

                memcpy(to_release, ctx->ctx->start_seq, ctx->ctx->parse_pos);
                APR_BRIGADE_INSERT_TAIL(pass_bb,
                                        apr_bucket_pool_create(to_release,
                                        ctx->ctx->parse_pos, ctx->ctx->pool,
                                        f->c->bucket_alloc));
            }
        }
        else if (PARSE_PRE_HEAD != ctx->state) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                          "SSI directive was not properly finished at the end "
                          "of parsed document %s", r->filename);
            if (ctx->ctx->flags & FLAG_PRINTING) {
                SSI_CREATE_ERROR_BUCKET(ctx->ctx, f, pass_bb);
            }
        }

        if (!(ctx->ctx->flags & FLAG_PRINTING)) {
            ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r,
                          "missing closing endif directive in parsed document"
                          " %s", r->filename);
        }

        /* cleanup our temporary memory */
        apr_brigade_destroy(ctx->tmp_bb);
        apr_pool_destroy(ctx->dpool);

        /* don't forget to finally insert the EOS bucket */
        APR_BRIGADE_INSERT_TAIL(pass_bb, b);
    }

    /* if something's left over, pass it along */
    if (!APR_BRIGADE_EMPTY(pass_bb)) {
        rv = ap_pass_brigade(f->next, pass_bb);
    }
    else {
        rv = APR_SUCCESS;
    }

    apr_brigade_destroy(pass_bb);
    return rv;
}

static void *create_includes_dir_config(apr_pool_t *p, char *dummy)
{
    include_dir_config *result =
        (include_dir_config *)apr_palloc(p, sizeof(include_dir_config));
