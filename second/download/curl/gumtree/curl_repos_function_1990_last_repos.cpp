static bool tailmatch(const char *cooke_domain, const char *hostname)
{
  size_t cookie_domain_len = strlen(cooke_domain);
  size_t hostname_len = strlen(hostname);

  if(hostname_len < cookie_domain_len)
    return FALSE;

  if(!strcasecompare(cooke_domain, hostname + hostname_len-cookie_domain_len))
    return FALSE;

  /* A lead char of cookie_domain is not '.'.
     RFC6265 4.1.2.3. The Domain Attribute says:
       For example, if the value of the Domain attribute is
       "example.com", the user agent will include the cookie in the Cookie
       header when making HTTP requests to example.com, www.example.com, and
       www.corp.example.com.
   */
  if(hostname_len == cookie_domain_len)
    return TRUE;
  if('.' == *(hostname + hostname_len - cookie_domain_len - 1))
    return TRUE;
  return FALSE;
}