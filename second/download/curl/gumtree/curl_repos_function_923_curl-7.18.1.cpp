static size_t
buffer_write(struct krb4buffer *buf, void *data, size_t len)
{
  if(buf->index + len > buf->size) {
    void *tmp;
    if(buf->data == NULL)
      tmp = malloc(1024);
    else
      tmp = realloc(buf->data, buf->index + len);
    if(tmp == NULL)
      return -1;
    buf->data = tmp;
    buf->size = buf->index + len;
  }
  memcpy((char*)buf->data + buf->index, data, len);
  buf->index += len;
  return len;
}