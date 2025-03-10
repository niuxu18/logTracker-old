int main(int argc, char **argv)
{
  char *URL;
  if(argc< 2 ) {
    fprintf(stderr, "Pass URL as argument please\n");
    return 1;
  }
  if(argc>2)
    arg2=argv[2];

  URL = argv[1]; /* provide this to the rest */

  fprintf(stderr, "URL: %s\n", URL);

#ifdef MALLOCDEBUG
  curl_memdebug("memdump");
#endif
  return test(URL);
}