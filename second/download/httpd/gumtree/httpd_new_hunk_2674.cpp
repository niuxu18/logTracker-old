            /* nasty, but needed to avoid confusing the browser
             */
            apr_table_unset(f->r->headers_out, "Content-Length");
        }
    }

    if (APLOGrtrace1(f->r)) {
        ap_log_rerror(APLOG_MARK, APLOG_TRACE1, 0, f->r,
                      "%sfiltering `%s' of type `%s' through `%s', cfg %s",
                      ctx->noop ? "NOT " : "",
                      f->r->uri ? f->r->uri : f->r->filename,
                      f->r->content_type ? f->r->content_type : "(unspecified)",
                      ctx->filter->command,
                      get_cfg_string(dc, ctx->filter, f->r->pool));
