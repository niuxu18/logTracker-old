int main(void)
{
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.example.com/");

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wrfu);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);

    res = curl_easy_perform(curl);

    if(!res) {
      struct curl_certinfo *certinfo;

      res = curl_easy_getinfo(curl, CURLINFO_CERTINFO, &certinfo);

      if(!res && certinfo) {
        int i;

        printf("%d certs!\n", certinfo->num_of_certs);

        for(i = 0; i < certinfo->num_of_certs; i++) {
          struct curl_slist *slist;

          for(slist = certinfo->certinfo[i]; slist; slist = slist->next)
            printf("%s\n", slist->data);

        }
      }

    }

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}