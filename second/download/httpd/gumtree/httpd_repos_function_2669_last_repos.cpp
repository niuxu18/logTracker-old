apr_status_t h2_proxy_fifo_push(h2_proxy_fifo *fifo, void *elem)
{
    return fifo_push(fifo, elem, 1);
}