int main(int argc, char *argv[])
{
    apr_pool_t *p;
    apr_socket_t *sock;
    apr_status_t rv;
    apr_sockaddr_t *remote_sa;

    apr_initialize();
    atexit(apr_terminate);
    apr_pool_create(&p, NULL);

    if (argc < 2) {
        exit(-1);
    }

    rv = apr_sockaddr_info_get(&remote_sa, "127.0.0.1", APR_UNSPEC, 8021, 0, p);
    if (rv != APR_SUCCESS) {
        exit(-1);
    }

    if (apr_socket_create(&sock, remote_sa->family, SOCK_STREAM, 0,
                p) != APR_SUCCESS) {
        exit(-1);
    }

    rv = apr_socket_timeout_set(sock, apr_time_from_sec(3));
    if (rv) {
        exit(-1);
    }

    apr_socket_connect(sock, remote_sa);
        
    if (!strcmp("read", argv[1])) {
        char datarecv[STRLEN];
        apr_size_t length = STRLEN;
        apr_status_t rv;

        memset(datarecv, 0, STRLEN);
        rv = apr_socket_recv(sock, datarecv, &length);
        apr_socket_close(sock);
        if (APR_STATUS_IS_TIMEUP(rv)) {
            exit(SOCKET_TIMEOUT); 
        }

        if (strcmp(datarecv, DATASTR)) {
            exit(-1);
        }
        
        exit(length);
    }
    else if (!strcmp("write", argv[1])) {
        apr_size_t length = strlen(DATASTR);
        apr_socket_send(sock, DATASTR, &length);

        apr_socket_close(sock);
        exit(length);
    }
    exit(-1);
}