static int get_pair(const char *str, char *value, char *content,
                    const char **endptr)
{
  int c;
  bool starts_with_quote = FALSE;
  bool escape = FALSE;

  for(c=MAX_VALUE_LENGTH-1; (*str && (*str != '=') && c--); )
    *value++ = *str++;
  *value=0;

  if('=' != *str++)
    /* eek, no match */
    return 1;

  if('\"' == *str) {
    /* this starts with a quote so it must end with one as well! */
    str++;
    starts_with_quote = TRUE;
  }

  for(c=MAX_CONTENT_LENGTH-1; *str && c--; str++) {
    switch(*str) {
    case '\\':
      if(!escape) {
        /* possibly the start of an escaped quote */
        escape = TRUE;
        *content++ = '\\'; /* even though this is an escape character, we still
                              store it as-is in the target buffer */
        continue;
      }
      break;
    case ',':
      if(!starts_with_quote) {
        /* this signals the end of the content if we didn't get a starting
           quote and then we do "sloppy" parsing */
        c=0; /* the end */
        continue;
      }
      break;
    case '\r':
    case '\n':
      /* end of string */
      c=0;
      continue;
    case '\"':
      if(!escape && starts_with_quote) {
        /* end of string */
        c=0;
        continue;
      }
      break;
    }
    escape = FALSE;
    *content++ = *str;
  }
  *content=0;

  *endptr = str;

  return 0; /* all is fine! */
}