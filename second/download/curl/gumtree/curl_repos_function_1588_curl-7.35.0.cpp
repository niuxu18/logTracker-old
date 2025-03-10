CURLcode Curl_base64_encode(struct SessionHandle *data,
                            const char *inputbuff, size_t insize,
                            char **outptr, size_t *outlen)
{
  CURLcode error;
  unsigned char ibuf[3];
  unsigned char obuf[4];
  int i;
  int inputparts;
  char *output;
  char *base64data;
  char *convbuf = NULL;

  const char *indata = inputbuff;

  *outptr = NULL;
  *outlen = 0;

  if(0 == insize)
    insize = strlen(indata);

  base64data = output = malloc(insize*4/3+4);
  if(NULL == output)
    return CURLE_OUT_OF_MEMORY;

  /*
   * The base64 data needs to be created using the network encoding
   * not the host encoding.  And we can't change the actual input
   * so we copy it to a buffer, translate it, and use that instead.
   */
  error = Curl_convert_clone(data, indata, insize, &convbuf);
  if(error) {
    free(output);
    return error;
  }

  if(convbuf)
    indata = (char *)convbuf;

  while(insize > 0) {
    for(i = inputparts = 0; i < 3; i++) {
      if(insize > 0) {
        inputparts++;
        ibuf[i] = (unsigned char) *indata;
        indata++;
        insize--;
      }
      else
        ibuf[i] = 0;
    }

    obuf[0] = (unsigned char)  ((ibuf[0] & 0xFC) >> 2);
    obuf[1] = (unsigned char) (((ibuf[0] & 0x03) << 4) | \
                               ((ibuf[1] & 0xF0) >> 4));
    obuf[2] = (unsigned char) (((ibuf[1] & 0x0F) << 2) | \
                               ((ibuf[2] & 0xC0) >> 6));
    obuf[3] = (unsigned char)   (ibuf[2] & 0x3F);

    switch(inputparts) {
    case 1: /* only one byte read */
      snprintf(output, 5, "%c%c==",
               table64[obuf[0]],
               table64[obuf[1]]);
      break;
    case 2: /* two bytes read */
      snprintf(output, 5, "%c%c%c=",
               table64[obuf[0]],
               table64[obuf[1]],
               table64[obuf[2]]);
      break;
    default:
      snprintf(output, 5, "%c%c%c%c",
               table64[obuf[0]],
               table64[obuf[1]],
               table64[obuf[2]],
               table64[obuf[3]] );
      break;
    }
    output += 4;
  }
  *output = '\0';
  *outptr = base64data; /* return pointer to new data, allocated memory */

  if(convbuf)
    free(convbuf);

  *outlen = strlen(base64data); /* return the length of the new data */

  return CURLE_OK;
}