            ap_rprintf(r, "</td><td>%" APR_TIME_T_FMT "</td>",
                apr_time_sec(balancer->s->timeout));
            ap_rprintf(r, "<td>%d</td>\n", balancer->s->max_attempts);
            ap_rprintf(r, "<td>%s</td>\n",
                       balancer->s->lbpname);
            ap_rputs("<td>", r);
            if (balancer->s->vhost && *(balancer->s->vhost)) {
                ap_rvputs(r, balancer->s->vhost, " -> ", NULL);
            }
            ap_rvputs(r, balancer->s->vpath, "</td>\n", NULL);
            ap_rprintf(r, "<td>%s</td>\n",
                       !balancer->s->inactive ? "Yes" : "No");
            ap_rputs("</table>\n<br />", r);
