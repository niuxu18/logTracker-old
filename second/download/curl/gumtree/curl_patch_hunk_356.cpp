   if(data)
     free(convbuf);
 #endif
   return strlen(base64data); /* return the length of the new data */
 }
 /* ---- End of Base64 Encoding ---- */
-
-/************* TEST HARNESS STUFF ****************/
-
-
-#ifdef TEST_ENCODE
-/* encoding test harness. Read in standard input and write out the length
- * returned by Curl_base64_encode, followed by the base64'd data itself
- */
-#include <stdio.h>
-
-#define TEST_NEED_SUCK
-void *suck(int *);
-
-int main(int argc, argv_item_t argv[], char **envp)
-{
-  char *base64;
-  size_t base64Len;
-  unsigned char *data;
-  int dataLen;
-  struct SessionHandle *handle = NULL;
-
-#ifdef CURL_DOES_CONVERSIONS
-  /* get a Curl handle so Curl_base64_encode can translate properly */
-  handle = curl_easy_init();
-  if(handle == NULL) {
-    fprintf(stderr, "Error: curl_easy_init failed\n");
-    return 1;
-  }
-#endif
-  data = (unsigned char *)suck(&dataLen);
-  base64Len = Curl_base64_encode(handle, data, dataLen, &base64);
-
-  fprintf(stderr, "%zu\n", base64Len);
-  fprintf(stdout, "%s\n", base64);
-
-  free(base64); free(data);
-#ifdef CURL_DOES_CONVERSIONS
-  curl_easy_cleanup(handle);
-#endif
-  return 0;
-}
-#endif
-
-#ifdef TEST_DECODE
-/* decoding test harness. Read in a base64 string from stdin and write out the
- * length returned by Curl_base64_decode, followed by the decoded data itself
- *
- * gcc -DTEST_DECODE base64.c -o base64 mprintf.o memdebug.o
- */
-#include <stdio.h>
-
-#define TEST_NEED_SUCK
-void *suck(int *);
-
-int main(int argc, argv_item_t argv[], char **envp)
-{
-  char *base64;
-  int base64Len;
-  unsigned char *data;
-  int dataLen;
-  int i, j;
-#ifdef CURL_DOES_CONVERSIONS
-  /* get a Curl handle so main can translate properly */
-  struct SessionHandle *handle = curl_easy_init();
-  if(handle == NULL) {
-    fprintf(stderr, "Error: curl_easy_init failed\n");
-    return 1;
-  }
-#endif
-
-  base64 = (char *)suck(&base64Len);
-  dataLen = Curl_base64_decode(base64, &data);
-
-  fprintf(stderr, "%d\n", dataLen);
-
-  for(i=0; i < dataLen; i+=0x10) {
-    printf("0x%02x: ", i);
-    for(j=0; j < 0x10; j++)
-      if((j+i) < dataLen)
-        printf("%02x ", data[i+j]);
-      else
-        printf("   ");
-
-    printf(" | ");
-
-    for(j=0; j < 0x10; j++)
-      if((j+i) < dataLen) {
-#ifdef CURL_DOES_CONVERSIONS
-        if(CURLE_OK !=
-             Curl_convert_from_network(handle, &data[i+j], (size_t)1))
-          data[i+j] = '.';
-#endif /* CURL_DOES_CONVERSIONS */
-        printf("%c", ISGRAPH(data[i+j])?data[i+j]:'.');
-      } else
-        break;
-    puts("");
-  }
-
-#ifdef CURL_DOES_CONVERSIONS
-  curl_easy_cleanup(handle);
-#endif
-  free(base64); free(data);
-  return 0;
-}
-#endif
-
-#ifdef TEST_NEED_SUCK
-/* this function 'sucks' in as much as possible from stdin */
-void *suck(int *lenptr)
-{
-  int cursize = 8192;
-  unsigned char *buf = NULL;
-  int lastread;
-  int len = 0;
-
-  do {
-    cursize *= 2;
-    buf = realloc(buf, cursize);
-    memset(buf + len, 0, cursize - len);
-    lastread = fread(buf + len, 1, cursize - len, stdin);
-    len += lastread;
-  } while(!feof(stdin));
-
-  lenptr[0] = len;
-  return (void *)buf;
-}
-#endif
