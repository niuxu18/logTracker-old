{
  char** new_argv;
  size_t size;
  char* s;
  int i;

  if (argc <= 0)
    return argv;

  /* Calculate how much memory we need for the argv strings. */
  size = 0;
  for (i = 0; i < argc; i++)
    size += strlen(argv[i]) + 1;

#if defined(__MVS__)
  /* argv is not adjacent. So just use argv[0] */
  process_title.str = argv[0];
  process_title.len = strlen(argv[0]);
#else
  process_title.str = argv[0];
  process_title.len = argv[argc - 1] + strlen(argv[argc - 1]) - argv[0];
  assert(process_title.len + 1 == size);  /* argv memory should be adjacent. */
#endif

  /* Add space for the argv pointers. */
  size += (argc + 1) * sizeof(char*);

  new_argv = uv__malloc(size);
  if (new_argv == NULL)
    return argv;
  args_mem = new_argv;

  /* Copy over the strings and set up the pointer table. */
  s = (char*) &new_argv[argc + 1];
  for (i = 0; i < argc; i++) {
    size = strlen(argv[i]) + 1;
    memcpy(s, argv[i], size);
    new_argv[i] = s;
    s += size;
  }
  new_argv[i] = NULL;

  return new_argv;
}