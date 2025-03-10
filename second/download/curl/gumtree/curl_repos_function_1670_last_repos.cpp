static CURLcode pausewrite(struct Curl_easy *data,
                           int type, /* what type of data */
                           const char *ptr,
                           size_t len)
{
  /* signalled to pause sending on this connection, but since we have data
     we want to send we need to dup it to save a copy for when the sending
     is again enabled */
  struct SingleRequest *k = &data->req;
  struct UrlState *s = &data->state;
  char *dupl;
  unsigned int i;
  bool newtype = TRUE;

  if(s->tempcount) {
    for(i = 0; i< s->tempcount; i++) {
      if(s->tempwrite[i].type == type) {
        /* data for this type exists */
        newtype = FALSE;
        break;
      }
    }
    DEBUGASSERT(i < 3);
  }
  else
    i = 0;

  if(!newtype) {
    /* append new data to old data */

    /* figure out the new size of the data to save */
    size_t newlen = len + s->tempwrite[i].len;
    /* allocate the new memory area */
    char *newptr = realloc(s->tempwrite[i].buf, newlen);
    if(!newptr)
      return CURLE_OUT_OF_MEMORY;
    /* copy the new data to the end of the new area */
    memcpy(newptr + s->tempwrite[i].len, ptr, len);

    /* update the pointer and the size */
    s->tempwrite[i].buf = newptr;
    s->tempwrite[i].len = newlen;
  }
  else {
    dupl = Curl_memdup(ptr, len);
    if(!dupl)
      return CURLE_OUT_OF_MEMORY;

    /* store this information in the state struct for later use */
    s->tempwrite[i].buf = dupl;
    s->tempwrite[i].len = len;
    s->tempwrite[i].type = type;

    if(newtype)
      s->tempcount++;
  }

  /* mark the connection as RECV paused */
  k->keepon |= KEEP_RECV_PAUSE;

  DEBUGF(infof(data, "Paused %zu bytes in buffer for type %02x\n",
               len, type));

  return CURLE_OK;
}