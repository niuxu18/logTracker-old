static int ascent_init (void) /* {{{ */
{
  static char credentials[1024];

  if (url == NULL)
  {
    WARNING ("ascent plugin: ascent_init: No URL configured, "
        "returning an error.");
    return (-1);
  }

  if (curl != NULL)
  {
    curl_easy_cleanup (curl);
  }

  if ((curl = curl_easy_init ()) == NULL)
  {
    ERROR ("ascent plugin: ascent_init: curl_easy_init failed.");
    return (-1);
  }

  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, ascent_curl_callback);
  curl_easy_setopt (curl, CURLOPT_USERAGENT, PACKAGE_NAME"/"PACKAGE_VERSION);
  curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, ascent_curl_error);

  if (user != NULL)
  {
    int status;

    status = ssnprintf (credentials, sizeof (credentials), "%s:%s",
        user, (pass == NULL) ? "" : pass);
    if (status >= sizeof (credentials))
    {
      ERROR ("ascent plugin: ascent_init: Returning an error because the "
          "credentials have been truncated.");
      return (-1);
    }

    curl_easy_setopt (curl, CURLOPT_USERPWD, credentials);
  }

  curl_easy_setopt (curl, CURLOPT_URL, url);

  if ((verify_peer == NULL) || (strcmp (verify_peer, "true") == 0))
    curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 1);
  else
    curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);

  if ((verify_host == NULL) || (strcmp (verify_host, "true") == 0))
    curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 2);
  else
    curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);

  if (cacert != NULL)
    curl_easy_setopt (curl, CURLOPT_CAINFO, cacert);

  return (0);
}