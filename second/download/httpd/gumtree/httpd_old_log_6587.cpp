ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, m->c,
                          "h2_mplx(%ld): start release_join with %d streams in hold", 
                          m->id, (int)h2_ihash_count(m->shold));