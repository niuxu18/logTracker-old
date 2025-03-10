                 " border-color: gray;\n"
                 " background-color: white;\n"
                 " text-align: center;\n"
                 "}\n"
                 "</style>\n</head>\n", r);
        ap_rputs("<body><h1>Load Balancer Manager for ", r);
        ap_rvputs(r, ap_get_server_name(r), "</h1>\n\n", NULL);
        ap_rvputs(r, "<dl><dt>Server Version: ",
                  ap_get_server_description(), "</dt>\n", NULL);
        ap_rvputs(r, "<dt>Server Built: ",
                  ap_get_server_built(), "\n</dt></dl>\n", NULL);
        balancer = (proxy_balancer *)conf->balancers->elts;
        for (i = 0; i < conf->balancers->nelts; i++) {

            ap_rputs("<hr />\n<h3>LoadBalancer Status for ", r);
            ap_rvputs(r, "<a href='", r->uri, "?b=",
                      balancer->s->name + sizeof(BALANCER_PREFIX) - 1,
                      "&nonce=", balancer->s->nonce,
                      "'>", NULL);
            ap_rvputs(r, balancer->s->name, "</a></h3>\n", NULL);
            ap_rputs("\n\n<table><tr>"
                "<th>MaxMembers</th><th>StickySession</th><th>DisableFailover</th><th>Timeout</th><th>FailoverAttempts</th><th>Method</th>"
                "<th>Path</th><th>Active</th></tr>\n<tr>", r);
            /* the below is a safe cast, since the number of slots total will
             * never be more than max_workers, which is restricted to int */
            ap_rprintf(r, "<td>%d [%d Used]</td>\n", balancer->max_workers,
