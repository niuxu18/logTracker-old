static void PTRCALL
utf8_toUtf8(const ENCODING *enc,
            const char **fromP, const char *fromLim,
            char **toP, const char *toLim)
{
  char *to;
  const char *from;
  if (fromLim - *fromP > toLim - *toP) {
    /* Avoid copying partial characters. */
    for (fromLim = *fromP + (toLim - *toP); fromLim > *fromP; fromLim--)
      if (((unsigned char)fromLim[-1] & 0xc0) != 0x80)
        break;
  }
  for (to = *toP, from = *fromP; from != fromLim; from++, to++)
    *to = *from;
  *fromP = from;
  *toP = to;
}