static const char *
    set_max_forwards(cmd_parms *parms, void *dummy, const char *arg)
{
    proxy_server_conf *psf =
    ap_get_module_config(parms->server->module_config, &proxy_module);
    long s = atol(arg);
    if (s < 0) {
        return "ProxyMaxForwards must be greater or equal to zero..";
    }

    psf->maxfwd = s;
    psf->maxfwd_set = 1;
    return NULL;
}