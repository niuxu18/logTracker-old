{
  char** arg;
  WCHAR* dst = NULL;
  WCHAR* temp_buffer = NULL;
  size_t dst_len = 0;
  size_t temp_buffer_len = 0;
  WCHAR* pos;
  int arg_count = 0;
  int err = 0;

  /* Count the required size. */
  for (arg = args; *arg; arg++) {
    DWORD arg_len;

    arg_len = MultiByteToWideChar(CP_UTF8,
                                  0,
                                  *arg,
                                  -1,
                                  NULL,
                                  0);
    if (arg_len == 0) {
      return GetLastError();
    }

    dst_len += arg_len;

    if (arg_len > temp_buffer_len)
      temp_buffer_len = arg_len;

    arg_count++;
  }

  /* Adjust for potential quotes. Also assume the worst-case scenario */
  /* that every character needs escaping, so we need twice as much space. */
  dst_len = dst_len * 2 + arg_count * 2;

  /* Allocate buffer for the final command line. */
  dst = (WCHAR*) uv__malloc(dst_len * sizeof(WCHAR));
  if (dst == NULL) {
    err = ERROR_OUTOFMEMORY;
    goto error;
  }

  /* Allocate temporary working buffer. */
  temp_buffer = (WCHAR*) uv__malloc(temp_buffer_len * sizeof(WCHAR));
  if (temp_buffer == NULL) {
    err = ERROR_OUTOFMEMORY;
    goto error;
  }

  pos = dst;
  for (arg = args; *arg; arg++) {
    DWORD arg_len;

    /* Convert argument to wide char. */
    arg_len = MultiByteToWideChar(CP_UTF8,
                                  0,
                                  *arg,
                                  -1,
                                  temp_buffer,
                                  (int) (dst + dst_len - pos));
    if (arg_len == 0) {
      err = GetLastError();
      goto error;
    }

    if (verbatim_arguments) {
      /* Copy verbatim. */
      wcscpy(pos, temp_buffer);
      pos += arg_len - 1;
    } else {
      /* Quote/escape, if needed. */
      pos = quote_cmd_arg(temp_buffer, pos);
    }

    *pos++ = *(arg + 1) ? L' ' : L'\0';
  }

  uv__free(temp_buffer);

  *dst_ptr = dst;
  return 0;

error:
  uv__free(dst);
  uv__free(temp_buffer);
  return err;
}