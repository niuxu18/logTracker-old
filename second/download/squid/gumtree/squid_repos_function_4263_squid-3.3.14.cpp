inline
int SSL_set_fd(SSL *ssl, int fd)
{
    return ::SSL_set_fd(ssl, _get_osfhandle(fd));
}