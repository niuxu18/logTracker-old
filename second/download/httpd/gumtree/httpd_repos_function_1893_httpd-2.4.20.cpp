static int find_ct(request_rec *r)
{
    mime_dir_config *conf;
    apr_array_header_t *exception_list;
    char *ext;
    const char *fn, *fntmp, *type, *charset = NULL, *resource_name;
    int found_metadata = 0;

    if (r->finfo.filetype == APR_DIR) {
        ap_set_content_type(r, DIR_MAGIC_TYPE);
        return OK;
    }

    if (!r->filename) {
        return DECLINED;
    }

    conf = (mime_dir_config *)ap_get_module_config(r->per_dir_config,
                                                   &mime_module);
    exception_list = apr_array_make(r->pool, 2, sizeof(char *));

    /* If use_path_info is explicitly set to on (value & 1 == 1), append. */
    if (conf->use_path_info & 1) {
        resource_name = apr_pstrcat(r->pool, r->filename, r->path_info, NULL);
    }
    else {
        resource_name = r->filename;
    }

    /* Always drop the path leading up to the file name.
     */
    if ((fn = ap_strrchr_c(resource_name, '/')) == NULL) {
        fn = resource_name;
    }
    else {
        ++fn;
    }


    /* The exception list keeps track of those filename components that
     * are not associated with extensions indicating metadata.
     * The base name is always the first exception (i.e., "txt.html" has
     * a basename of "txt" even though it might look like an extension).
     * Leading dots are considered to be part of the base name (a file named
     * ".png" is likely not a png file but just a hidden file called png).
     */
    fntmp = fn;
    while (*fntmp == '.')
        fntmp++;
    fntmp = ap_strchr_c(fntmp, '.');
    if (fntmp) {
        ext = apr_pstrmemdup(r->pool, fn, fntmp - fn);
        fn = fntmp + 1;
    }
    else {
        ext = apr_pstrdup(r->pool, fn);
        fn += strlen(fn);
    }

    *((const char **)apr_array_push(exception_list)) = ext;

    /* Parse filename extensions which can be in any order
     */
    while (*fn && (ext = ap_getword(r->pool, &fn, '.'))) {
        const extension_info *exinfo = NULL;
        int found;
        char *extcase;

        if (*ext == '\0') {  /* ignore empty extensions "bad..html" */
            continue;
        }

        found = 0;

        /* Save the ext in extcase before converting it to lower case.
         */
        extcase = apr_pstrdup(r->pool, ext);
        ap_str_tolower(ext);

        if (conf->extension_mappings != NULL) {
            exinfo = (extension_info*)apr_hash_get(conf->extension_mappings,
                                                   ext, APR_HASH_KEY_STRING);
        }

        if (exinfo == NULL || !exinfo->forced_type) {
            if ((type = apr_hash_get(mime_type_extensions, ext,
                                     APR_HASH_KEY_STRING)) != NULL) {
                ap_set_content_type(r, (char*) type);
                found = 1;
            }
        }

        if (exinfo != NULL) {

            /* empty string is treated as special case for RemoveType */
            if (exinfo->forced_type && *exinfo->forced_type) {
                ap_set_content_type(r, exinfo->forced_type);
                found = 1;
            }

            if (exinfo->charset_type) {
                charset = exinfo->charset_type;
                found = 1;
            }
            if (exinfo->language_type) {
                if (!r->content_languages) {
                    r->content_languages = apr_array_make(r->pool, 2,
                                                          sizeof(char *));
                }
                *((const char **)apr_array_push(r->content_languages))
                    = exinfo->language_type;
                found = 1;
            }
            if (exinfo->encoding_type) {
                if (!r->content_encoding) {
                    r->content_encoding = exinfo->encoding_type;
                }
                else {
                    /* XXX should eliminate duplicate entities
                     *
                     * ah no. Order is important and double encoding is neither
                     * forbidden nor impossible. -- nd
                     */
                    r->content_encoding = apr_pstrcat(r->pool,
                                                      r->content_encoding,
                                                      ", ",
                                                      exinfo->encoding_type,
                                                      NULL);
                }
                found = 1;
            }
            /* The following extensions are not 'Found'.  That is, they don't
             * make any contribution to metadata negotiation, so they must have
             * been explicitly requested by name.
             */
            if (exinfo->handler && r->proxyreq == PROXYREQ_NONE) {
                r->handler = exinfo->handler;
                if (conf->multimatch & MULTIMATCH_HANDLERS) {
                    found = 1;
                }
            }
            /* XXX Two significant problems; 1, we don't check to see if we are
             * setting redundant filters.    2, we insert these in the types
             * config hook, which may be too early (dunno.)
             */
            if (exinfo->input_filters) {
                const char *filter, *filters = exinfo->input_filters;
                while (*filters
                    && (filter = ap_getword(r->pool, &filters, ';'))) {
                    ap_add_input_filter(filter, NULL, r, r->connection);
                }
                if (conf->multimatch & MULTIMATCH_FILTERS) {
                    found = 1;
                }
            }
            if (exinfo->output_filters) {
                const char *filter, *filters = exinfo->output_filters;
                while (*filters
                    && (filter = ap_getword(r->pool, &filters, ';'))) {
                    ap_add_output_filter(filter, NULL, r, r->connection);
                }
                if (conf->multimatch & MULTIMATCH_FILTERS) {
                    found = 1;
                }
            }
        }

        if (found || (conf->multimatch & MULTIMATCH_ANY)) {
            found_metadata = 1;
        }
        else {
            *((const char **) apr_array_push(exception_list)) = extcase;
        }
    }

    /*
     * Need to set a notes entry on r for unrecognized elements.
     * Somebody better claim them!  If we did absolutely nothing,
     * skip the notes to alert mod_negotiation we are clueless.
     */
    if (found_metadata) {
        apr_table_setn(r->notes, "ap-mime-exceptions-list",
                       (void *)exception_list);
    }

    if (r->content_type) {
        content_type *ctp;
        int override = 0;

        if ((ctp = analyze_ct(r, r->content_type))) {
            param *pp = ctp->param;
            char *base_content_type = apr_palloc(r->pool, ctp->type_len +
                                                 ctp->subtype_len +
                                                 sizeof("/"));
            char *tmp = base_content_type;
            memcpy(tmp, ctp->type, ctp->type_len);
            tmp += ctp->type_len;
            *tmp++ = '/';
            memcpy(tmp, ctp->subtype, ctp->subtype_len);
            tmp += ctp->subtype_len;
            *tmp = 0;
            ap_set_content_type(r, base_content_type);
            while (pp != NULL) {
                if (charset && !strcmp(pp->attr, "charset")) {
                    if (!override) {
                        ap_set_content_type(r,
                                            apr_pstrcat(r->pool,
                                                        r->content_type,
                                                        "; charset=",
                                                        charset,
                                                        NULL));
                        override = 1;
                    }
                }
                else {
                    ap_set_content_type(r,
                                        apr_pstrcat(r->pool,
                                                    r->content_type,
                                                    "; ", pp->attr,
                                                    "=", pp->val,
                                                    NULL));
                }
                pp = pp->next;
            }
            if (charset && !override) {
                ap_set_content_type(r, apr_pstrcat(r->pool, r->content_type,
                                                   "; charset=", charset,
                                                   NULL));
            }
        }
    }

    /* Set default language, if none was specified by the extensions
     * and we have a DefaultLanguage setting in force
     */

    if (!r->content_languages && conf->default_language) {
        const char **new;

        if (!r->content_languages) {
            r->content_languages = apr_array_make(r->pool, 2, sizeof(char *));
        }
        new = (const char **)apr_array_push(r->content_languages);
        *new = conf->default_language;
    }

    if (!r->content_type) {
        return DECLINED;
    }

    return OK;
}