void
Curl_freeaddrinfo(Curl_addrinfo *cahead)
{
  Curl_addrinfo *vqualifier canext;
  Curl_addrinfo *ca;

  for(ca = cahead; ca != NULL; ca = canext) {

    if(ca->ai_addr)
      free(ca->ai_addr);

    if(ca->ai_canonname)
      free(ca->ai_canonname);

    canext = ca->ai_next;

    free(ca);
  }
}