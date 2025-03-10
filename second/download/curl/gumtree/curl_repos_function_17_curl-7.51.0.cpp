int main(int argc, char *argv[])
{
  CURL *curl_handle;
  CURLcode res;
  int prtall = 0, prtsep = 0, prttime = 0;
  const char *url = URL_1M;
  char *appname = argv[0];

  if(argc > 1) {
    /* parse input parameters */
    for(argc--, argv++; *argv; argc--, argv++) {
      if(strncasecmp(*argv, "-", 1) == 0) {
        if(strncasecmp(*argv, "-H", 2) == 0) {
          fprintf(stderr,
                  "\rUsage: %s [-m=1|2|5|10|20|50|100] [-t] [-x] [url]\n",
                  appname);
          exit(1);
        }
        else if(strncasecmp(*argv, "-V", 2) == 0) {
          fprintf(stderr, "\r%s %s - %s\n",
                  appname, CHKSPEED_VERSION, curl_version());
          exit(1);
        }
        else if(strncasecmp(*argv, "-A", 2) == 0) {
          prtall = 1;
        }
        else if(strncasecmp(*argv, "-X", 2) == 0) {
          prtsep = 1;
        }
        else if(strncasecmp(*argv, "-T", 2) == 0) {
          prttime = 1;
        }
        else if(strncasecmp(*argv, "-M=", 3) == 0) {
          long m = strtol((*argv)+3, NULL, 10);
          switch(m) {
          case 1:
            url = URL_1M;
            break;
          case 2:
            url = URL_2M;
            break;
          case 5:
            url = URL_5M;
            break;
          case 10:
            url = URL_10M;
            break;
          case 20:
            url = URL_20M;
            break;
          case 50:
            url = URL_50M;
            break;
          case 100:
            url = URL_100M;
            break;
          default:
            fprintf(stderr, "\r%s: invalid parameter %s\n",
                    appname, *argv + 3);
            exit(1);
          }
        }
        else {
          fprintf(stderr, "\r%s: invalid or unknown option %s\n",
                  appname, *argv);
          exit(1);
        }
      }
      else {
        url = *argv;
      }
    }
  }

  /* print separator line */
  if(prtsep) {
    printf("-------------------------------------------------\n");
  }
  /* print localtime */
  if(prttime) {
    time_t t = time(NULL);
    printf("Localtime: %s", ctime(&t));
  }

  /* init libcurl */
  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT,
                   "libcurl-speedchecker/" CHKSPEED_VERSION);

  /* get it! */
  res = curl_easy_perform(curl_handle);

  if(CURLE_OK == res) {
    double val;

    /* check for bytes downloaded */
    res = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD, &val);
    if((CURLE_OK == res) && (val>0))
      printf("Data downloaded: %0.0f bytes.\n", val);

    /* check for total download time */
    res = curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME, &val);
    if((CURLE_OK == res) && (val>0))
      printf("Total download time: %0.3f sec.\n", val);

    /* check for average download speed */
    res = curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD, &val);
    if((CURLE_OK == res) && (val>0))
      printf("Average download speed: %0.3f kbyte/sec.\n", val / 1024);

    if(prtall) {
      /* check for name resolution time */
      res = curl_easy_getinfo(curl_handle, CURLINFO_NAMELOOKUP_TIME, &val);
      if((CURLE_OK == res) && (val>0))
        printf("Name lookup time: %0.3f sec.\n", val);

      /* check for connect time */
      res = curl_easy_getinfo(curl_handle, CURLINFO_CONNECT_TIME, &val);
      if((CURLE_OK == res) && (val>0))
        printf("Connect time: %0.3f sec.\n", val);
    }
  }
  else {
    fprintf(stderr, "Error while fetching '%s' : %s\n",
            url, curl_easy_strerror(res));
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  /* we're done with libcurl, so clean it up */
  curl_global_cleanup();

  return 0;
}