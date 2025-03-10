apr_status_t apr_socket_timeout_set(apr_socket_t *sock, apr_interval_time_t t)
{
    apr_status_t stat;

    /* If our timeout is positive or zero and our last timeout was
     * negative, then we need to ensure that we are non-blocking.
     * Conversely, if our timeout is negative and we had a positive
     * or zero timeout, we must make sure our socket is blocking.
     * We want to avoid calling fcntl more than necessary on the socket,
     */
    if (t >= 0 && sock->timeout < 0) {
        if (apr_is_option_set(sock->netmask, APR_SO_NONBLOCK) != 1) {
            if ((stat = sononblock(sock->socketdes)) != APR_SUCCESS) {
                return stat;
            }
        }
    } 
    else if (t < 0 && sock->timeout >= 0) {
        if (apr_is_option_set(sock->netmask, APR_SO_NONBLOCK) != 0) { 
            if ((stat = soblock(sock->socketdes)) != APR_SUCCESS) { 
                return stat; 
            }
        } 
    }
    /* must disable the incomplete read support if we change to a
     * blocking socket.
     */
    if (t == 0) {
        sock->netmask &= ~APR_INCOMPLETE_READ;
    }
    sock->timeout = t; 
    apr_set_option(&sock->netmask, APR_SO_TIMEOUT, t);
    return APR_SUCCESS;
}