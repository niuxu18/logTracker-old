void Curl_md4it(unsigned char *output, const unsigned char *input, size_t len)
{
  MD4_CTX ctx;
  MD4Init(&ctx);
  MD4Update(&ctx, input, curlx_uztoui(len));
  MD4Final(output, &ctx);
}