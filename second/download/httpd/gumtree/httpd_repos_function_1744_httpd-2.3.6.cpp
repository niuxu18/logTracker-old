static void * merge_proxy_config(apr_pool_t *p, void *basev, void *overridesv)
{
    proxy_server_conf *ps = apr_pcalloc(p, sizeof(proxy_server_conf));
    proxy_server_conf *base = (proxy_server_conf *) basev;
    proxy_server_conf *overrides = (proxy_server_conf *) overridesv;

    ps->proxies = apr_array_append(p, base->proxies, overrides->proxies);
    ps->sec_proxy = apr_array_append(p, base->sec_proxy, overrides->sec_proxy);
    ps->aliases = apr_array_append(p, base->aliases, overrides->aliases);
    ps->noproxies = apr_array_append(p, base->noproxies, overrides->noproxies);
    ps->dirconn = apr_array_append(p, base->dirconn, overrides->dirconn);
    ps->workers = apr_array_append(p, base->workers, overrides->workers);
    ps->balancers = apr_array_append(p, base->balancers, overrides->balancers);
    ps->forward = overrides->forward ? overrides->forward : base->forward;
    ps->reverse = overrides->reverse ? overrides->reverse : base->reverse;

    ps->domain = (overrides->domain == NULL) ? base->domain : overrides->domain;
    ps->viaopt = (overrides->viaopt_set == 0) ? base->viaopt : overrides->viaopt;
    ps->viaopt_set = overrides->viaopt_set || base->viaopt_set;
    ps->req = (overrides->req_set == 0) ? base->req : overrides->req;
    ps->req_set = overrides->req_set || base->req_set;
    ps->recv_buffer_size = (overrides->recv_buffer_size_set == 0) ? base->recv_buffer_size : overrides->recv_buffer_size;
    ps->recv_buffer_size_set = overrides->recv_buffer_size_set || base->recv_buffer_size_set;
    ps->io_buffer_size = (overrides->io_buffer_size_set == 0) ? base->io_buffer_size : overrides->io_buffer_size;
    ps->io_buffer_size_set = overrides->io_buffer_size_set || base->io_buffer_size_set;
    ps->maxfwd = (overrides->maxfwd_set == 0) ? base->maxfwd : overrides->maxfwd;
    ps->maxfwd_set = overrides->maxfwd_set || base->maxfwd_set;
    ps->error_override = (overrides->error_override_set == 0) ? base->error_override : overrides->error_override;
    ps->error_override_set = overrides->error_override_set || base->error_override_set;
    ps->timeout= (overrides->timeout_set == 0) ? base->timeout : overrides->timeout;
    ps->timeout_set = overrides->timeout_set || base->timeout_set;
    ps->badopt = (overrides->badopt_set == 0) ? base->badopt : overrides->badopt;
    ps->badopt_set = overrides->badopt_set || base->badopt_set;
    ps->proxy_status = (overrides->proxy_status_set == 0) ? base->proxy_status : overrides->proxy_status;
    ps->proxy_status_set = overrides->proxy_status_set || base->proxy_status_set;
    ps->pool = p;
    return ps;
}