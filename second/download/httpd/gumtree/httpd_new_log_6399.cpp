ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, shed->c, APLOGNO(03400)
                  "h2_ngn_shed(%ld): task %s %s by %s", 
                  shed->c->id, task->id, aborted? "aborted":"done", ngn->id);