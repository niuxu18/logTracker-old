ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, m->c,
                  "h2_mplx(%ld): destroy, refs=%d", 
                  m->id, m->refs);