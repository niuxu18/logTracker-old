apr_status_t apr_os_sock_make(apr_socket_t **apr_sock, 
                              apr_os_sock_info_t *os_sock_info, 
                              apr_pool_t *cont)
{
    alloc_socket(apr_sock, cont);
    set_socket_vars(*apr_sock, os_sock_info->family, os_sock_info->type);
    (*apr_sock)->timeout = -1;
    (*apr_sock)->socketdes = *os_sock_info->os_sock;
    if (os_sock_info->local) {
        memcpy(&(*apr_sock)->local_addr->sa.sin, 
               os_sock_info->local, 
               (*apr_sock)->local_addr->salen);
        /* XXX IPv6 - this assumes sin_port and sin6_port at same offset */
        (*apr_sock)->local_addr->port = ntohs((*apr_sock)->local_addr->sa.sin.sin_port);
    }
    else {
        (*apr_sock)->local_port_unknown = (*apr_sock)->local_interface_unknown = 1;
    }
    if (os_sock_info->remote) {
#ifndef HAVE_POLL
        (*apr_sock)->connected = 1;
#endif
        memcpy(&(*apr_sock)->remote_addr->sa.sin, 
               os_sock_info->remote,
               (*apr_sock)->remote_addr->salen);
        /* XXX IPv6 - this assumes sin_port and sin6_port at same offset */
        (*apr_sock)->remote_addr->port = ntohs((*apr_sock)->remote_addr->sa.sin.sin_port);
    }
        
    (*apr_sock)->inherit = 0;
    apr_pool_cleanup_register((*apr_sock)->cntxt, (void *)(*apr_sock), 
                              socket_cleanup, socket_cleanup);
    return APR_SUCCESS;
}