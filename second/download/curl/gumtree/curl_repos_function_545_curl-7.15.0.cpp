int main(int argc, char **argv, char **envp)
{
  char *base64;
  size_t base64Len;
  unsigned char *data;
  int dataLen;

  data = (unsigned char *)suck(&dataLen);
  base64Len = Curl_base64_encode(data, dataLen, &base64);

  fprintf(stderr, "%d\n", base64Len);
  fprintf(stdout, "%s",   base64);

  free(base64); free(data);
  return 0;
}