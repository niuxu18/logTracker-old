bool curl_win32_idn_to_ascii(const char *in, char **out)
{
  bool success = FALSE;

  wchar_t *in_w = Curl_convert_UTF8_to_wchar(in);
  if(in_w) {
    wchar_t punycode[IDN_MAX_LENGTH];
    int chars = IdnToAscii(0, in_w, -1, punycode, IDN_MAX_LENGTH);
    free(in_w);
    if(chars) {
      *out = Curl_convert_wchar_to_UTF8(punycode);
      if(*out)
        success = TRUE;
    }
  }

  return success;
}