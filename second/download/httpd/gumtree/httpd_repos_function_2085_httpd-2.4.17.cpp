static void socket_cleanup(proxy_conn_rec *conn)
{
    conn->sock = NULL;
    conn->connection = NULL;
    conn->ssl_hostname = NULL;
    apr_pool_clear(conn->scpool);
}