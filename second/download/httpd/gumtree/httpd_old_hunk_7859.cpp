        set_neg_headers(r, neg, na_list);
        store_variant_list(r, neg);
        return MULTIPLE_CHOICES;
    }

    if (!best) {
        ap_log_error(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r->server,
                    "no acceptable variant: %s", r->filename);

        set_neg_headers(r, neg, na_result);
        store_variant_list(r, neg);
        return NOT_ACCEPTABLE;
    }
