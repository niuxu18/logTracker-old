static
void utf8_toUtf16(const ENCODING *enc,
		  const char **fromP, const char *fromLim,
		  unsigned short **toP, const unsigned short *toLim)
{
  unsigned short *to = *toP;
  const char *from = *fromP;
  while (from != fromLim && to != toLim) {
    switch (((struct normal_encoding *)enc)->type[(unsigned char)*from]) {
    case BT_LEAD2:
      *to++ = ((from[0] & 0x1f) << 6) | (from[1] & 0x3f);
      from += 2;
      break;
    case BT_LEAD3:
      *to++ = ((from[0] & 0xf) << 12) | ((from[1] & 0x3f) << 6) | (from[2] & 0x3f);
      from += 3;
      break;
    case BT_LEAD4:
      {
	unsigned long n;
	if (to + 1 == toLim)
	  break;
	n = ((from[0] & 0x7) << 18) | ((from[1] & 0x3f) << 12) | ((from[2] & 0x3f) << 6) | (from[3] & 0x3f);
	n -= 0x10000;
	to[0] = (unsigned short)((n >> 10) | 0xD800);
	to[1] = (unsigned short)((n & 0x3FF) | 0xDC00);
	to += 2;
	from += 4;
      }
      break;
    default:
      *to++ = *from++;
      break;
    }
  }
  *fromP = from;
  *toP = to;
}