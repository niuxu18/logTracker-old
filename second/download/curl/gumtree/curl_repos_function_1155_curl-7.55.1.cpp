static char *imap_atom(const char *str, bool escape_only)
{
  /* !checksrc! disable PARENBRACE 1 */
  const char atom_specials[] = "(){ %*]";
  const char *p1;
  char *p2;
  size_t backsp_count = 0;
  size_t quote_count = 0;
  bool others_exists = FALSE;
  size_t newlen = 0;
  char *newstr = NULL;

  if(!str)
    return NULL;

  /* Look for "atom-specials", counting the backslash and quote characters as
     these will need escapping */
  p1 = str;
  while(*p1) {
    if(*p1 == '\\')
      backsp_count++;
    else if(*p1 == '"')
      quote_count++;
    else if(!escape_only) {
      const char *p3 = atom_specials;

      while(*p3 && !others_exists) {
        if(*p1 == *p3)
          others_exists = TRUE;

        p3++;
      }
    }

    p1++;
  }

  /* Does the input contain any "atom-special" characters? */
  if(!backsp_count && !quote_count && !others_exists)
    return strdup(str);

  /* Calculate the new string length */
  newlen = strlen(str) + backsp_count + quote_count + (others_exists ? 2 : 0);

  /* Allocate the new string */
  newstr = (char *) malloc((newlen + 1) * sizeof(char));
  if(!newstr)
    return NULL;

  /* Surround the string in quotes if necessary */
  p2 = newstr;
  if(others_exists) {
    newstr[0] = '"';
    newstr[newlen - 1] = '"';
    p2++;
  }

  /* Copy the string, escaping backslash and quote characters along the way */
  p1 = str;
  while(*p1) {
    if(*p1 == '\\' || *p1 == '"') {
      *p2 = '\\';
      p2++;
    }

   *p2 = *p1;

    p1++;
    p2++;
  }

  /* Terminate the string */
  newstr[newlen] = '\0';

  return newstr;
}