int test(char *URL)
{
  int res;
  CURL *curl;
  int request=1;
  char *stream_uri = NULL;

  if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  if((curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "curl_easy_init() failed\n");
    curl_global_cleanup();
    return TEST_ERR_MAJOR_BAD;
  }

  test_setopt(curl, CURLOPT_HEADERDATA, stdout);
  test_setopt(curl, CURLOPT_WRITEDATA, stdout);
  test_setopt(curl, CURLOPT_VERBOSE, 1L);

  test_setopt(curl, CURLOPT_URL, URL);

  test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_OPTIONS);

  if((stream_uri = suburl(URL, request++)) == NULL) {
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
  free(stream_uri);
  stream_uri = NULL;

  res = curl_easy_perform(curl);
  if(res != (int)CURLE_RTSP_CSEQ_ERROR) {
    fprintf(stderr, "Failed to detect CSeq mismatch");
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }

  test_setopt(curl, CURLOPT_RTSP_CLIENT_CSEQ, 999L);
  test_setopt(curl, CURLOPT_RTSP_TRANSPORT,
                    "RAW/RAW/UDP;unicast;client_port=3056-3057");
  test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_SETUP);

  if((stream_uri = suburl(URL, request++)) == NULL) {
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
  free(stream_uri);
  stream_uri = NULL;

  res = curl_easy_perform(curl);
  if(res)
    goto test_cleanup;

  test_setopt(curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_PLAY);

  if((stream_uri = suburl(URL, request++)) == NULL) {
    res = TEST_ERR_MAJOR_BAD;
    goto test_cleanup;
  }
  test_setopt(curl, CURLOPT_RTSP_STREAM_URI, stream_uri);
  free(stream_uri);
  stream_uri = NULL;

  res = curl_easy_perform(curl);
  if(res != CURLE_RTSP_SESSION_ERROR) {
    fprintf(stderr, "Failed to detect a Session ID mismatch");
  }

test_cleanup:
  free(stream_uri);

  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return res;
}