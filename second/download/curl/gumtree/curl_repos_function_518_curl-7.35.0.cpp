int test(char *URL)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  FILE *hd_src ;
  int hd ;
  struct_stat file_info;
  struct curl_slist *hl;
  int error;

  struct curl_slist *headerlist=NULL;
  const char *buf_1 = "RNFR 505";
  const char *buf_2 = "RNTO 505-forreal";

  if (!libtest_arg2) {
    fprintf(stderr, "Usage: <url> <file-to-upload>\n");
    return -1;
  }

  hd_src = fopen(libtest_arg2, "rb");
  if(NULL == hd_src) {
    error = ERRNO;
    fprintf(stderr, "fopen() failed with error: %d %s\n",
            error, strerror(error));
    fprintf(stderr, "Error opening file: %s\n", libtest_arg2);
    return -2; /* if this happens things are major weird */
  }

  /* get the file size of the local file */
  hd = fstat(fileno(hd_src), &file_info);
  if(hd == -1) {
    /* can't open file, bail out */
    error = ERRNO;
    fprintf(stderr, "fstat() failed with error: %d %s\n",
            error, strerror(error));
    fprintf(stderr, "ERROR: cannot open file %s\n", libtest_arg2);
    fclose(hd_src);
    return -1;
  }

  if(! file_info.st_size) {
    fprintf(stderr, "ERROR: file %s has zero size!\n", libtest_arg2);
    fclose(hd_src);
    return -4;
  }

  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed\n");
    fclose(hd_src);
    return TEST_ERR_MAJOR_BAD;
  }

  /* get a curl handle */
  if ((curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "curl_easy_init() failed\n");
    curl_global_cleanup();
    fclose(hd_src);
    return TEST_ERR_MAJOR_BAD;
  }

  /* build a list of commands to pass to libcurl */

  if ((hl = curl_slist_append(headerlist, buf_1)) == NULL) {
    fprintf(stderr, "curl_slist_append() failed\n");
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    fclose(hd_src);
    return TEST_ERR_MAJOR_BAD;
  }
  if ((headerlist = curl_slist_append(hl, buf_2)) == NULL) {
    fprintf(stderr, "curl_slist_append() failed\n");
    curl_slist_free_all(hl);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    fclose(hd_src);
    return TEST_ERR_MAJOR_BAD;
  }
  headerlist = hl;

  /* enable uploading */
  test_setopt(curl, CURLOPT_UPLOAD, 1L);

  /* enable verbose */
  test_setopt(curl, CURLOPT_VERBOSE, 1L);

  /* specify target */
  test_setopt(curl,CURLOPT_URL, URL);

  /* pass in that last of FTP commands to run after the transfer */
  test_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

  /* now specify which file to upload */
  test_setopt(curl, CURLOPT_INFILE, hd_src);

  /* and give the size of the upload (optional) */
  test_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                   (curl_off_t)file_info.st_size);

  /* Now run off and do what you've been told! */
  res = curl_easy_perform(curl);

test_cleanup:

  /* clean up the FTP commands list */
  curl_slist_free_all(headerlist);

  /* close the local file */
  fclose(hd_src);

  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return res;
}