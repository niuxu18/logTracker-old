static
void latin1_toUtf8(const ENCODING *enc,
		   const char **fromP, const char *fromLim,
		   char **toP, const char *toLim)
{
  for (;;) {
    unsigned char c;
    if (*fromP == fromLim)
      break;
    c = (unsigned char)**fromP;
    if (c & 0x80) {
      if (toLim - *toP < 2)
	break;
      *(*toP)++ = ((c >> 6) | UTF8_cval2);
      *(*toP)++ = ((c & 0x3f) | 0x80);
      (*fromP)++;
    }
    else {
      if (*toP == toLim)
	break;
      *(*toP)++ = *(*fromP)++;
    }
  }
}