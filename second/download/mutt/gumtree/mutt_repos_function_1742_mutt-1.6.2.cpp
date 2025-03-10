int getdnsdomainname (char *d, size_t len)
{
  /* A DNS name can actually be only 253 octets, string is 256 */
  char *node;
  long node_len;
  struct addrinfo hints;
  struct addrinfo *h;
  char *p;
  int ret;

  *d = '\0';
  memset(&hints, 0, sizeof (struct addrinfo));
  hints.ai_flags = AI_CANONNAME;
  hints.ai_family = AF_UNSPEC;

  if ((node_len = sysconf(_SC_HOST_NAME_MAX)) == -1)
    node_len = STRING;
  node = safe_malloc(node_len + 1);
  if (gethostname(node, node_len))
    ret = -1;
  else if (getaddrinfo(node, NULL, &hints, &h))
    ret = -1;
  else
  {
    if (!(p = strchr(h->ai_canonname, '.')))
      ret = -1;
    else
    {
      strfcpy(d, ++p, len);
      ret = 0;
      dprint(1, (debugfile, "getdnsdomainname(): %s\n", d));
    }
    freeaddrinfo(h);
  }
  FREE (&node);
  return ret;
}