  int n;

  fputs (t1, f);

  /* don't try to press string into one line with less than 40 characters.
     The double parenthesis avoids a gcc warning, sigh ... */
  if ((split = COLS < 40))
  {
    col_a = col = 0;
    col_b = LONG_STRING;
    fputc ('\n', f);
  }
  else
  {
    col_a = COLS > 83 ? (COLS - 32) >> 2 : 12;
    col_b = COLS > 49 ? (COLS - 10) >> 1 : 19;
    col = pad (f, mutt_strwidth(t1), col_a);
  }

  if (ismacro > 0)
  {
    if (!mutt_strcmp (Pager, "builtin"))
