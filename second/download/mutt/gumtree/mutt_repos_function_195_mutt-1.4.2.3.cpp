char *mutt_iconv_hook (const char *chs)
{
  return _mutt_string_hook (chs, M_ICONVHOOK);
}