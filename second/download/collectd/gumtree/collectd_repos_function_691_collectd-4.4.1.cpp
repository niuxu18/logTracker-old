int match_apply (cu_match_t *obj, const char *str)
{
  int status;
  regmatch_t re_match[32];
  char *matches[32];
  size_t matches_num;
  int i;

  if ((obj == NULL) || (str == NULL))
    return (-1);

  status = regexec (&obj->regex, str,
      STATIC_ARRAY_SIZE (re_match), re_match,
      /* eflags = */ 0);

  /* Regex did not match */
  if (status != 0)
    return (0);

  memset (matches, '\0', sizeof (matches));
  for (matches_num = 0; matches_num < STATIC_ARRAY_SIZE (matches); matches_num++)
  {
    if ((re_match[matches_num].rm_so < 0)
	|| (re_match[matches_num].rm_eo < 0))
      break;

    matches[matches_num] = match_substr (str,
	re_match[matches_num].rm_so, re_match[matches_num].rm_eo);
    if (matches[matches_num] == NULL)
    {
      status = -1;
      break;
    }
  }

  if (status != 0)
  {
    ERROR ("utils_match: match_apply: match_substr failed.");
  }
  else
  {
    status = obj->callback (str, matches, matches_num, obj->user_data);
    if (status != 0)
    {
      ERROR ("utils_match: match_apply: callback failed.");
    }
  }

  for (i = 0; i < matches_num; i++)
  {
    sfree (matches[i]);
  }

  return (status);
}