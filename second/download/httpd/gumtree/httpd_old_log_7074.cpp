ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(01174)
                              "%s: Forcing worker (%s) into error state " 
                              "due to status code %d matching 'failonstatus' "
                              "balancer parameter",
                              balancer->s->name, worker->s->name, val);