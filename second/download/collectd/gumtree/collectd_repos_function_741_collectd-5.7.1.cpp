static int ps_read_process(long pid, procstat_t *ps, char *state) {
  char filename[64];
  char buffer[1024];

  char *fields[64];
  char fields_len;

  size_t buffer_len;

  char *buffer_ptr;
  size_t name_start_pos;
  size_t name_end_pos;
  size_t name_len;

  derive_t cpu_user_counter;
  derive_t cpu_system_counter;
  long long unsigned vmem_size;
  long long unsigned vmem_rss;
  long long unsigned stack_size;

  ssize_t status;

  memset(ps, 0, sizeof(procstat_t));

  ssnprintf(filename, sizeof(filename), "/proc/%li/stat", pid);

  status = read_file_contents(filename, buffer, sizeof(buffer) - 1);
  if (status <= 0)
    return (-1);
  buffer_len = (size_t)status;
  buffer[buffer_len] = 0;

  /* The name of the process is enclosed in parens. Since the name can
   * contain parens itself, spaces, numbers and pretty much everything
   * else, use these to determine the process name. We don't use
   * strchr(3) and strrchr(3) to avoid pointer arithmetic which would
   * otherwise be required to determine name_len. */
  name_start_pos = 0;
  while (name_start_pos < buffer_len && buffer[name_start_pos] != '(')
    name_start_pos++;

  name_end_pos = buffer_len;
  while (name_end_pos > 0 && buffer[name_end_pos] != ')')
    name_end_pos--;

  /* Either '(' or ')' is not found or they are in the wrong order.
   * Anyway, something weird that shouldn't happen ever. */
  if (name_start_pos >= name_end_pos) {
    ERROR("processes plugin: name_start_pos = %zu >= name_end_pos = %zu",
          name_start_pos, name_end_pos);
    return (-1);
  }

  name_len = (name_end_pos - name_start_pos) - 1;
  if (name_len >= sizeof(ps->name))
    name_len = sizeof(ps->name) - 1;

  sstrncpy(ps->name, &buffer[name_start_pos + 1], name_len + 1);

  if ((buffer_len - name_end_pos) < 2)
    return (-1);
  buffer_ptr = &buffer[name_end_pos + 2];

  fields_len = strsplit(buffer_ptr, fields, STATIC_ARRAY_SIZE(fields));
  if (fields_len < 22) {
    DEBUG("processes plugin: ps_read_process (pid = %li):"
          " `%s' has only %i fields..",
          pid, filename, fields_len);
    return (-1);
  }

  *state = fields[0][0];

  if (*state == 'Z') {
    ps->num_lwp = 0;
    ps->num_proc = 0;
  } else {
    ps->num_lwp = strtoul(fields[17], /* endptr = */ NULL, /* base = */ 10);
    if ((ps_read_status(pid, ps)) == NULL) {
      /* No VMem data */
      ps->vmem_data = -1;
      ps->vmem_code = -1;
      DEBUG("ps_read_process: did not get vmem data for pid %li", pid);
    }
    if (ps->num_lwp == 0)
      ps->num_lwp = 1;
    ps->num_proc = 1;
  }

  /* Leave the rest at zero if this is only a zombi */
  if (ps->num_proc == 0) {
    DEBUG("processes plugin: This is only a zombie: pid = %li; "
          "name = %s;",
          pid, ps->name);
    return (0);
  }

  cpu_user_counter = atoll(fields[11]);
  cpu_system_counter = atoll(fields[12]);
  vmem_size = atoll(fields[20]);
  vmem_rss = atoll(fields[21]);
  ps->vmem_minflt_counter = atol(fields[7]);
  ps->vmem_majflt_counter = atol(fields[9]);

  {
    unsigned long long stack_start = atoll(fields[25]);
    unsigned long long stack_ptr = atoll(fields[26]);

    stack_size = (stack_start > stack_ptr) ? stack_start - stack_ptr
                                           : stack_ptr - stack_start;
  }

  /* Convert jiffies to useconds */
  cpu_user_counter = cpu_user_counter * 1000000 / CONFIG_HZ;
  cpu_system_counter = cpu_system_counter * 1000000 / CONFIG_HZ;
  vmem_rss = vmem_rss * pagesize_g;

  ps->cpu_user_counter = cpu_user_counter;
  ps->cpu_system_counter = cpu_system_counter;
  ps->vmem_size = (unsigned long)vmem_size;
  ps->vmem_rss = (unsigned long)vmem_rss;
  ps->stack_size = (unsigned long)stack_size;

  /* success */
  return (0);
}