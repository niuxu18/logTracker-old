 
     dest[dest_pos] = src[src_pos];
     dest_pos++;
     src_pos++;
   }
 
-  assert (dest_pos <= (dest_size - 2));
+  assert(dest_pos <= (dest_size - 2));
 
   dest[dest_pos] = '"';
   dest_pos++;
 
   dest[dest_pos] = 0;
   dest_pos++;
   src_pos++;
 
   return (dest);
 } /* }}} char *lcc_strescape */
 
 /* lcc_chomp: Removes all control-characters at the end of a string. */
-static void lcc_chomp (char *str) /* {{{ */
+static void lcc_chomp(char *str) /* {{{ */
 {
   size_t str_len;
 
-  str_len = strlen (str);
-  while (str_len > 0)
-  {
+  str_len = strlen(str);
+  while (str_len > 0) {
     if (str[str_len - 1] >= 32)
       break;
     str[str_len - 1] = 0;
     str_len--;
   }
 } /* }}} void lcc_chomp */
 
-static void lcc_response_free (lcc_response_t *res) /* {{{ */
+static void lcc_response_free(lcc_response_t *res) /* {{{ */
 {
   if (res == NULL)
     return;
 
   for (size_t i = 0; i < res->lines_num; i++)
-    free (res->lines[i]);
-  free (res->lines);
+    free(res->lines[i]);
+  free(res->lines);
   res->lines = NULL;
 } /* }}} void lcc_response_free */
 
-static int lcc_send (lcc_connection_t *c, const char *command) /* {{{ */
+static int lcc_send(lcc_connection_t *c, const char *command) /* {{{ */
 {
   int status;
 
-  lcc_tracef ("send:    --> %s\n", command);
+  lcc_tracef("send:    --> %s\n", command);
 
-  status = fprintf (c->fh, "%s\r\n", command);
-  if (status < 0)
-  {
-    lcc_set_errno (c, errno);
+  status = fprintf(c->fh, "%s\r\n", command);
+  if (status < 0) {
+    lcc_set_errno(c, errno);
     return (-1);
   }
   fflush(c->fh);
 
   return (0);
 } /* }}} int lcc_send */
 
-static int lcc_receive (lcc_connection_t *c, /* {{{ */
-    lcc_response_t *ret_res)
-{
-  lcc_response_t res = { 0 };
+static int lcc_receive(lcc_connection_t *c, /* {{{ */
+                       lcc_response_t *ret_res) {
+  lcc_response_t res = {0};
   char *ptr;
   char buffer[4096];
   size_t i;
 
   /* Read the first line, containing the status and a message */
-  ptr = fgets (buffer, sizeof (buffer), c->fh);
-  if (ptr == NULL)
-  {
-    lcc_set_errno (c, errno);
+  ptr = fgets(buffer, sizeof(buffer), c->fh);
+  if (ptr == NULL) {
+    lcc_set_errno(c, errno);
     return (-1);
   }
-  lcc_chomp (buffer);
-  lcc_tracef ("receive: <-- %s\n", buffer);
+  lcc_chomp(buffer);
+  lcc_tracef("receive: <-- %s\n", buffer);
 
   /* Convert the leading status to an integer and make `ptr' to point to the
    * beginning of the message. */
   ptr = NULL;
   errno = 0;
-  res.status = (int) strtol (buffer, &ptr, 0);
-  if ((errno != 0) || (ptr == &buffer[0]))
-  {
-    lcc_set_errno (c, errno);
+  res.status = (int)strtol(buffer, &ptr, 0);
+  if ((errno != 0) || (ptr == &buffer[0])) {
+    lcc_set_errno(c, errno);
     return (-1);
   }
 
   /* Skip white spaces after the status number */
   while ((*ptr == ' ') || (*ptr == '\t'))
     ptr++;
 
   /* Now copy the message. */
-  strncpy (res.message, ptr, sizeof (res.message));
-  res.message[sizeof (res.message) - 1] = 0;
+  strncpy(res.message, ptr, sizeof(res.message));
+  res.message[sizeof(res.message) - 1] = 0;
 
   /* Error or no lines follow: We're done. */
-  if (res.status <= 0)
-  {
-    memcpy (ret_res, &res, sizeof (res));
+  if (res.status <= 0) {
+    memcpy(ret_res, &res, sizeof(res));
     return (0);
   }
 
   /* Allocate space for the char-pointers */
-  res.lines_num = (size_t) res.status;
+  res.lines_num = (size_t)res.status;
   res.status = 0;
-  res.lines = malloc (res.lines_num * sizeof (*res.lines));
-  if (res.lines == NULL)
-  {
-    lcc_set_errno (c, ENOMEM);
+  res.lines = malloc(res.lines_num * sizeof(*res.lines));
+  if (res.lines == NULL) {
+    lcc_set_errno(c, ENOMEM);
     return (-1);
   }
 
   /* Now receive all the lines */
-  for (i = 0; i < res.lines_num; i++)
-  {
-    ptr = fgets (buffer, sizeof (buffer), c->fh);
-    if (ptr == NULL)
-    {
-      lcc_set_errno (c, errno);
+  for (i = 0; i < res.lines_num; i++) {
+    ptr = fgets(buffer, sizeof(buffer), c->fh);
+    if (ptr == NULL) {
+      lcc_set_errno(c, errno);
       break;
     }
-    lcc_chomp (buffer);
-    lcc_tracef ("receive: <-- %s\n", buffer);
+    lcc_chomp(buffer);
+    lcc_tracef("receive: <-- %s\n", buffer);
 
-    res.lines[i] = strdup (buffer);
-    if (res.lines[i] == NULL)
-    {
-      lcc_set_errno (c, ENOMEM);
+    res.lines[i] = strdup(buffer);
+    if (res.lines[i] == NULL) {
+      lcc_set_errno(c, ENOMEM);
       break;
     }
   }
 
   /* Check if the for-loop exited with an error. */
-  if (i < res.lines_num)
-  {
-    while (i > 0)
-    {
+  if (i < res.lines_num) {
+    while (i > 0) {
       i--;
-      free (res.lines[i]);
+      free(res.lines[i]);
     }
-    free (res.lines);
+    free(res.lines);
     return (-1);
   }
 
-  memcpy (ret_res, &res, sizeof (res));
+  memcpy(ret_res, &res, sizeof(res));
   return (0);
 } /* }}} int lcc_receive */
 
-static int lcc_sendreceive (lcc_connection_t *c, /* {{{ */
-    const char *command, lcc_response_t *ret_res)
-{
-  lcc_response_t res = { 0 };
+static int lcc_sendreceive(lcc_connection_t *c, /* {{{ */
+                           const char *command, lcc_response_t *ret_res) {
+  lcc_response_t res = {0};
   int status;
 
-  if (c->fh == NULL)
-  {
-    lcc_set_errno (c, EBADF);
+  if (c->fh == NULL) {
+    lcc_set_errno(c, EBADF);
     return (-1);
   }
 
-  status = lcc_send (c, command);
+  status = lcc_send(c, command);
   if (status != 0)
     return (status);
 
-  status = lcc_receive (c, &res);
+  status = lcc_receive(c, &res);
   if (status == 0)
-    memcpy (ret_res, &res, sizeof (*ret_res));
+    memcpy(ret_res, &res, sizeof(*ret_res));
 
   return (status);
 } /* }}} int lcc_sendreceive */
 
-static int lcc_open_unixsocket (lcc_connection_t *c, const char *path) /* {{{ */
+static int lcc_open_unixsocket(lcc_connection_t *c, const char *path) /* {{{ */
 {
-  struct sockaddr_un sa = { 0 };
+  struct sockaddr_un sa = {0};
   int fd;
   int status;
 
-  assert (c != NULL);
-  assert (c->fh == NULL);
-  assert (path != NULL);
+  assert(c != NULL);
+  assert(c->fh == NULL);
+  assert(path != NULL);
 
   /* Don't use PF_UNIX here, because it's broken on Mac OS X (10.4, possibly
    * others). */
-  fd = socket (AF_UNIX, SOCK_STREAM, /* protocol = */ 0);
-  if (fd < 0)
-  {
-    lcc_set_errno (c, errno);
+  fd = socket(AF_UNIX, SOCK_STREAM, /* protocol = */ 0);
+  if (fd < 0) {
+    lcc_set_errno(c, errno);
     return (-1);
   }
 
   sa.sun_family = AF_UNIX;
-  strncpy (sa.sun_path, path, sizeof (sa.sun_path) - 1);
+  strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
 
-  status = connect (fd, (struct sockaddr *) &sa, sizeof (sa));
-  if (status != 0)
-  {
-    lcc_set_errno (c, errno);
-    close (fd);
+  status = connect(fd, (struct sockaddr *)&sa, sizeof(sa));
+  if (status != 0) {
+    lcc_set_errno(c, errno);
+    close(fd);
     return (-1);
   }
 
-  c->fh = fdopen (fd, "r+");
-  if (c->fh == NULL)
-  {
-    lcc_set_errno (c, errno);
-    close (fd);
+  c->fh = fdopen(fd, "r+");
+  if (c->fh == NULL) {
+    lcc_set_errno(c, errno);
+    close(fd);
     return (-1);
   }
 
   return (0);
 } /* }}} int lcc_open_unixsocket */
 
-static int lcc_open_netsocket (lcc_connection_t *c, /* {{{ */
-    const char *addr_orig)
-{
+static int lcc_open_netsocket(lcc_connection_t *c, /* {{{ */
+                              const char *addr_orig) {
   struct addrinfo *ai_res;
   char addr_copy[NI_MAXHOST];
   char *addr;
   char *port;
   int fd;
   int status;
 
-  assert (c != NULL);
-  assert (c->fh == NULL);
-  assert (addr_orig != NULL);
+  assert(c != NULL);
+  assert(c->fh == NULL);
+  assert(addr_orig != NULL);
 
   strncpy(addr_copy, addr_orig, sizeof(addr_copy));
   addr_copy[sizeof(addr_copy) - 1] = '\0';
   addr = addr_copy;
 
   port = NULL;
   if (*addr == '[') /* IPv6+port format */
   {
     /* `addr' is something like "[2001:780:104:2:211:24ff:feab:26f8]:12345" */
     addr++;
 
-    port = strchr (addr, ']');
-    if (port == NULL)
-    {
-      LCC_SET_ERRSTR (c, "malformed address: %s", addr_orig);
+    port = strchr(addr, ']');
+    if (port == NULL) {
+      LCC_SET_ERRSTR(c, "malformed address: %s", addr_orig);
       return (-1);
     }
     *port = 0;
     port++;
 
     if (*port == ':')
       port++;
     else if (*port == 0)
       port = NULL;
-    else
-    {
-      LCC_SET_ERRSTR (c, "garbage after address: %s", port);
+    else {
+      LCC_SET_ERRSTR(c, "garbage after address: %s", port);
       return (-1);
     }
-  } /* if (*addr = ']') */
-  else if (strchr (addr, '.') != NULL) /* Hostname or IPv4 */
+  }                                   /* if (*addr = ']') */
+  else if (strchr(addr, '.') != NULL) /* Hostname or IPv4 */
   {
-    port = strrchr (addr, ':');
-    if (port != NULL)
-    {
+    port = strrchr(addr, ':');
+    if (port != NULL) {
       *port = 0;
       port++;
     }
   }
 
-  struct addrinfo ai_hints = {
-    .ai_family = AF_UNSPEC,
-    .ai_flags = AI_ADDRCONFIG,
-    .ai_socktype = SOCK_STREAM
-  };
-
-  status = getaddrinfo (addr,
-                        port == NULL ? LCC_DEFAULT_PORT : port,
-                        &ai_hints, &ai_res);
-  if (status != 0)
-  {
-    LCC_SET_ERRSTR (c, "getaddrinfo: %s", gai_strerror (status));
+  struct addrinfo ai_hints = {.ai_family = AF_UNSPEC,
+                              .ai_flags = AI_ADDRCONFIG,
+                              .ai_socktype = SOCK_STREAM};
+
+  status = getaddrinfo(addr, port == NULL ? LCC_DEFAULT_PORT : port, &ai_hints,
+                       &ai_res);
+  if (status != 0) {
+    LCC_SET_ERRSTR(c, "getaddrinfo: %s", gai_strerror(status));
     return (-1);
   }
 
-  for (struct addrinfo *ai_ptr = ai_res; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next)
-  {
-    fd = socket (ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
-    if (fd < 0)
-    {
+  for (struct addrinfo *ai_ptr = ai_res; ai_ptr != NULL;
+       ai_ptr = ai_ptr->ai_next) {
+    fd = socket(ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
+    if (fd < 0) {
       status = errno;
       continue;
     }
 
-    status = connect (fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen);
-    if (status != 0)
-    {
+    status = connect(fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen);
+    if (status != 0) {
       status = errno;
-      close (fd);
+      close(fd);
       continue;
     }
 
-    c->fh = fdopen (fd, "r+");
-    if (c->fh == NULL)
-    {
+    c->fh = fdopen(fd, "r+");
+    if (c->fh == NULL) {
       status = errno;
-      close (fd);
+      close(fd);
       continue;
     }
 
-    assert (status == 0);
+    assert(status == 0);
     break;
   } /* for (ai_ptr) */
 
-  if (status != 0)
-  {
-    lcc_set_errno (c, status);
-    freeaddrinfo (ai_res);
+  if (status != 0) {
+    lcc_set_errno(c, status);
+    freeaddrinfo(ai_res);
     return (-1);
   }
 
-  freeaddrinfo (ai_res);
+  freeaddrinfo(ai_res);
   return (0);
 } /* }}} int lcc_open_netsocket */
 
-static int lcc_open_socket (lcc_connection_t *c, const char *addr) /* {{{ */
+static int lcc_open_socket(lcc_connection_t *c, const char *addr) /* {{{ */
 {
   int status = 0;
 
   if (addr == NULL)
     return (-1);
 
-  assert (c != NULL);
-  assert (c->fh == NULL);
-  assert (addr != NULL);
+  assert(c != NULL);
+  assert(c->fh == NULL);
+  assert(addr != NULL);
 
-  if (strncmp ("unix:", addr, strlen ("unix:")) == 0)
-    status = lcc_open_unixsocket (c, addr + strlen ("unix:"));
+  if (strncmp("unix:", addr, strlen("unix:")) == 0)
+    status = lcc_open_unixsocket(c, addr + strlen("unix:"));
   else if (addr[0] == '/')
-    status = lcc_open_unixsocket (c, addr);
+    status = lcc_open_unixsocket(c, addr);
   else
-    status = lcc_open_netsocket (c, addr);
+    status = lcc_open_netsocket(c, addr);
 
   return (status);
 } /* }}} int lcc_open_socket */
 
 /*
  * Public functions
  */
-unsigned int lcc_version (void) /* {{{ */
+unsigned int lcc_version(void) /* {{{ */
 {
   return (LCC_VERSION);
 } /* }}} unsigned int lcc_version */
 
-const char *lcc_version_string (void) /* {{{ */
+const char *lcc_version_string(void) /* {{{ */
 {
   return (LCC_VERSION_STRING);
 } /* }}} const char *lcc_version_string */
 
-const char *lcc_version_extra (void) /* {{{ */
+const char *lcc_version_extra(void) /* {{{ */
 {
   return (LCC_VERSION_EXTRA);
 } /* }}} const char *lcc_version_extra */
 
-int lcc_connect (const char *address, lcc_connection_t **ret_con) /* {{{ */
+int lcc_connect(const char *address, lcc_connection_t **ret_con) /* {{{ */
 {
   lcc_connection_t *c;
   int status;
 
   if (address == NULL)
     return (-1);
 
   if (ret_con == NULL)
     return (-1);
 
-  c = calloc (1, sizeof (*c));
+  c = calloc(1, sizeof(*c));
   if (c == NULL)
     return (-1);
 
-  status = lcc_open_socket (c, address);
-  if (status != 0)
-  {
-    lcc_disconnect (c);
+  status = lcc_open_socket(c, address);
+  if (status != 0) {
+    lcc_disconnect(c);
     return (status);
   }
 
   *ret_con = c;
   return (0);
 } /* }}} int lcc_connect */
 
-int lcc_disconnect (lcc_connection_t *c) /* {{{ */
+int lcc_disconnect(lcc_connection_t *c) /* {{{ */
 {
   if (c == NULL)
     return (-1);
 
-  if (c->fh != NULL)
-  {
-    fclose (c->fh);
+  if (c->fh != NULL) {
+    fclose(c->fh);
     c->fh = NULL;
   }
 
-  free (c);
+  free(c);
   return (0);
 } /* }}} int lcc_disconnect */
 
-int lcc_getval (lcc_connection_t *c, lcc_identifier_t *ident, /* {{{ */
-    size_t *ret_values_num, gauge_t **ret_values, char ***ret_values_names)
-{
+int lcc_getval(lcc_connection_t *c, lcc_identifier_t *ident, /* {{{ */
+               size_t *ret_values_num, gauge_t **ret_values,
+               char ***ret_values_names) {
   char ident_str[6 * LCC_NAME_LEN];
   char ident_esc[12 * LCC_NAME_LEN];
   char command[14 * LCC_NAME_LEN];
 
   lcc_response_t res;
-  size_t   values_num;
+  size_t values_num;
   gauge_t *values = NULL;
-  char   **values_names = NULL;
+  char **values_names = NULL;
 
   size_t i;
   int status;
 
   if (c == NULL)
     return (-1);
 
-  if (ident == NULL)
-  {
-    lcc_set_errno (c, EINVAL);
+  if (ident == NULL) {
+    lcc_set_errno(c, EINVAL);
     return (-1);
   }
 
   /* Build a commend with an escaped version of the identifier string. */
-  status = lcc_identifier_to_string (c, ident_str, sizeof (ident_str), ident);
+  status = lcc_identifier_to_string(c, ident_str, sizeof(ident_str), ident);
   if (status != 0)
     return (status);
 
-  snprintf (command, sizeof (command), "GETVAL %s",
-      lcc_strescape (ident_esc, ident_str, sizeof (ident_esc)));
-  command[sizeof (command) - 1] = 0;
+  snprintf(command, sizeof(command), "GETVAL %s",
+           lcc_strescape(ident_esc, ident_str, sizeof(ident_esc)));
+  command[sizeof(command) - 1] = 0;
 
   /* Send talk to the daemon.. */
-  status = lcc_sendreceive (c, command, &res);
+  status = lcc_sendreceive(c, command, &res);
   if (status != 0)
     return (status);
 
-  if (res.status != 0)
-  {
-    LCC_SET_ERRSTR (c, "Server error: %s", res.message);
-    lcc_response_free (&res);
+  if (res.status != 0) {
+    LCC_SET_ERRSTR(c, "Server error: %s", res.message);
+    lcc_response_free(&res);
     return (-1);
   }
 
   values_num = res.lines_num;
 
-#define BAIL_OUT(e) do { \
-  lcc_set_errno (c, (e)); \
-  free (values); \
-  if (values_names != NULL) { \
-    for (i = 0; i < values_num; i++) { \
-      free (values_names[i]); \
-    } \
-  } \
-  free (values_names); \
-  lcc_response_free (&res); \
-  return (-1); \
-} while (0)
+#define BAIL_OUT(e)                                                            \
+  do {                                                                         \
+    lcc_set_errno(c, (e));                                                     \
+    free(values);                                                              \
+    if (values_names != NULL) {                                                \
+      for (i = 0; i < values_num; i++) {                                       \
+        free(values_names[i]);                                                 \
+      }                                                                        \
+    }                                                                          \
+    free(values_names);                                                        \
+    lcc_response_free(&res);                                                   \
+    return (-1);                                                               \
+  } while (0)
 
   /* If neither the values nor the names are requested, return here.. */
-  if ((ret_values == NULL) && (ret_values_names == NULL))
-  {
+  if ((ret_values == NULL) && (ret_values_names == NULL)) {
     if (ret_values_num != NULL)
       *ret_values_num = values_num;
-    lcc_response_free (&res);
+    lcc_response_free(&res);
     return (0);
   }
 
   /* Allocate space for the values */
-  if (ret_values != NULL)
-  {
-    values = malloc (values_num * sizeof (*values));
+  if (ret_values != NULL) {
+    values = malloc(values_num * sizeof(*values));
     if (values == NULL)
-      BAIL_OUT (ENOMEM);
+      BAIL_OUT(ENOMEM);
   }
 
-  if (ret_values_names != NULL)
-  {
-    values_names = calloc (values_num, sizeof (*values_names));
+  if (ret_values_names != NULL) {
+    values_names = calloc(values_num, sizeof(*values_names));
     if (values_names == NULL)
-      BAIL_OUT (ENOMEM);
+      BAIL_OUT(ENOMEM);
   }
 
-  for (i = 0; i < res.lines_num; i++)
-  {
+  for (i = 0; i < res.lines_num; i++) {
     char *key;
     char *value;
     char *endptr;
 
     key = res.lines[i];
-    value = strchr (key, '=');
+    value = strchr(key, '=');
     if (value == NULL)
-      BAIL_OUT (EILSEQ);
+      BAIL_OUT(EILSEQ);
 
     *value = 0;
     value++;
 
-    if (values != NULL)
-    {
+    if (values != NULL) {
       endptr = NULL;
       errno = 0;
-      values[i] = strtod (value, &endptr);
+      values[i] = strtod(value, &endptr);
 
       if ((endptr == value) || (errno != 0))
-        BAIL_OUT (errno);
+        BAIL_OUT(errno);
     }
 
-    if (values_names != NULL)
-    {
-      values_names[i] = strdup (key);
+    if (values_names != NULL) {
+      values_names[i] = strdup(key);
       if (values_names[i] == NULL)
-        BAIL_OUT (ENOMEM);
+        BAIL_OUT(ENOMEM);
     }
   } /* for (i = 0; i < res.lines_num; i++) */
 
   if (ret_values_num != NULL)
     *ret_values_num = values_num;
   if (ret_values != NULL)
     *ret_values = values;
   if (ret_values_names != NULL)
     *ret_values_names = values_names;
 
-  lcc_response_free (&res);
+  lcc_response_free(&res);
 
   return (0);
 } /* }}} int lcc_getval */
 
-int lcc_putval (lcc_connection_t *c, const lcc_value_list_t *vl) /* {{{ */
+int lcc_putval(lcc_connection_t *c, const lcc_value_list_t *vl) /* {{{ */
 {
   char ident_str[6 * LCC_NAME_LEN];
   char ident_esc[12 * LCC_NAME_LEN];
   char command[1024] = "";
   lcc_response_t res;
   int status;
 
-  if ((c == NULL) || (vl == NULL) || (vl->values_len < 1)
-      || (vl->values == NULL) || (vl->values_types == NULL))
-  {
-    lcc_set_errno (c, EINVAL);
+  if ((c == NULL) || (vl == NULL) || (vl->values_len < 1) ||
+      (vl->values == NULL) || (vl->values_types == NULL)) {
+    lcc_set_errno(c, EINVAL);
     return (-1);
   }
 
-  status = lcc_identifier_to_string (c, ident_str, sizeof (ident_str),
-      &vl->identifier);
+  status = lcc_identifier_to_string(c, ident_str, sizeof(ident_str),
+                                    &vl->identifier);
   if (status != 0)
     return (status);
 
-  SSTRCATF (command, "PUTVAL %s",
-      lcc_strescape (ident_esc, ident_str, sizeof (ident_esc)));
+  SSTRCATF(command, "PUTVAL %s",
+           lcc_strescape(ident_esc, ident_str, sizeof(ident_esc)));
 
   if (vl->interval > 0.0)
-    SSTRCATF (command, " interval=%.3f", vl->interval);
+    SSTRCATF(command, " interval=%.3f", vl->interval);
 
   if (vl->time > 0.0)
-    SSTRCATF (command, " %.3f", vl->time);
+    SSTRCATF(command, " %.3f", vl->time);
   else
-    SSTRCAT (command, " N");
+    SSTRCAT(command, " N");
 
-  for (size_t i = 0; i < vl->values_len; i++)
-  {
+  for (size_t i = 0; i < vl->values_len; i++) {
     if (vl->values_types[i] == LCC_TYPE_COUNTER)
-      SSTRCATF (command, ":%"PRIu64, vl->values[i].counter);
-    else if (vl->values_types[i] == LCC_TYPE_GAUGE)
-    {
-      if (isnan (vl->values[i].gauge))
-        SSTRCATF (command, ":U");
+      SSTRCATF(command, ":%" PRIu64, vl->values[i].counter);
+    else if (vl->values_types[i] == LCC_TYPE_GAUGE) {
+      if (isnan(vl->values[i].gauge))
+        SSTRCATF(command, ":U");
       else
-        SSTRCATF (command, ":%g", vl->values[i].gauge);
-    }
-    else if (vl->values_types[i] == LCC_TYPE_DERIVE)
-	SSTRCATF (command, ":%"PRIu64, vl->values[i].derive);
+        SSTRCATF(command, ":%g", vl->values[i].gauge);
+    } else if (vl->values_types[i] == LCC_TYPE_DERIVE)
+      SSTRCATF(command, ":%" PRIu64, vl->values[i].derive);
     else if (vl->values_types[i] == LCC_TYPE_ABSOLUTE)
-	SSTRCATF (command, ":%"PRIu64, vl->values[i].absolute);
+      SSTRCATF(command, ":%" PRIu64, vl->values[i].absolute);
 
   } /* for (i = 0; i < vl->values_len; i++) */
 
-  status = lcc_sendreceive (c, command, &res);
+  status = lcc_sendreceive(c, command, &res);
   if (status != 0)
     return (status);
 
-  if (res.status != 0)
-  {
-    LCC_SET_ERRSTR (c, "Server error: %s", res.message);
-    lcc_response_free (&res);
+  if (res.status != 0) {
+    LCC_SET_ERRSTR(c, "Server error: %s", res.message);
+    lcc_response_free(&res);
     return (-1);
   }
 
-  lcc_response_free (&res);
+  lcc_response_free(&res);
   return (0);
 } /* }}} int lcc_putval */
 
-int lcc_flush (lcc_connection_t *c, const char *plugin, /* {{{ */
-    lcc_identifier_t *ident, int timeout)
-{
+int lcc_flush(lcc_connection_t *c, const char *plugin, /* {{{ */
+              lcc_identifier_t *ident, int timeout) {
   char command[1024] = "";
   lcc_response_t res;
   int status;
 
-  if (c == NULL)
-  {
-    lcc_set_errno (c, EINVAL);
+  if (c == NULL) {
+    lcc_set_errno(c, EINVAL);
     return (-1);
   }
 
-  SSTRCPY (command, "FLUSH");
+  SSTRCPY(command, "FLUSH");
 
   if (timeout > 0)
-    SSTRCATF (command, " timeout=%i", timeout);
+    SSTRCATF(command, " timeout=%i", timeout);
 
-  if (plugin != NULL)
-  {
+  if (plugin != NULL) {
     char buffer[2 * LCC_NAME_LEN];
-    SSTRCATF (command, " plugin=%s",
-        lcc_strescape (buffer, plugin, sizeof (buffer)));
+    SSTRCATF(command, " plugin=%s",
+             lcc_strescape(buffer, plugin, sizeof(buffer)));
   }
 
-  if (ident != NULL)
-  {
+  if (ident != NULL) {
     char ident_str[6 * LCC_NAME_LEN];
     char ident_esc[12 * LCC_NAME_LEN];
 
-    status = lcc_identifier_to_string (c, ident_str, sizeof (ident_str), ident);
+    status = lcc_identifier_to_string(c, ident_str, sizeof(ident_str), ident);
     if (status != 0)
       return (status);
 
-    SSTRCATF (command, " identifier=%s",
-        lcc_strescape (ident_esc, ident_str, sizeof (ident_esc)));
+    SSTRCATF(command, " identifier=%s",
+             lcc_strescape(ident_esc, ident_str, sizeof(ident_esc)));
   }
 
-  status = lcc_sendreceive (c, command, &res);
+  status = lcc_sendreceive(c, command, &res);
   if (status != 0)
     return (status);
 
-  if (res.status != 0)
-  {
-    LCC_SET_ERRSTR (c, "Server error: %s", res.message);
-    lcc_response_free (&res);
+  if (res.status != 0) {
+    LCC_SET_ERRSTR(c, "Server error: %s", res.message);
+    lcc_response_free(&res);
     return (-1);
   }
 
-  lcc_response_free (&res);
+  lcc_response_free(&res);
   return (0);
 } /* }}} int lcc_flush */
 
 /* TODO: Implement lcc_putnotif */
 
-int lcc_listval (lcc_connection_t *c, /* {{{ */
-    lcc_identifier_t **ret_ident, size_t *ret_ident_num)
-{
+int lcc_listval(lcc_connection_t *c, /* {{{ */
+                lcc_identifier_t **ret_ident, size_t *ret_ident_num) {
   lcc_response_t res;
   int status;
 
   lcc_identifier_t *ident;
   size_t ident_num;
 
   if (c == NULL)
     return (-1);
 
-  if ((ret_ident == NULL) || (ret_ident_num == NULL))
-  {
-    lcc_set_errno (c, EINVAL);
+  if ((ret_ident == NULL) || (ret_ident_num == NULL)) {
+    lcc_set_errno(c, EINVAL);
     return (-1);
   }
 
-  status = lcc_sendreceive (c, "LISTVAL", &res);
+  status = lcc_sendreceive(c, "LISTVAL", &res);
   if (status != 0)
     return (status);
 
-  if (res.status != 0)
-  {
-    LCC_SET_ERRSTR (c, "Server error: %s", res.message);
-    lcc_response_free (&res);
+  if (res.status != 0) {
+    LCC_SET_ERRSTR(c, "Server error: %s", res.message);
+    lcc_response_free(&res);
     return (-1);
   }
 
   ident_num = res.lines_num;
-  ident = malloc (ident_num * sizeof (*ident));
-  if (ident == NULL)
-  {
-    lcc_response_free (&res);
-    lcc_set_errno (c, ENOMEM);
+  ident = malloc(ident_num * sizeof(*ident));
+  if (ident == NULL) {
+    lcc_response_free(&res);
+    lcc_set_errno(c, ENOMEM);
     return (-1);
   }
 
-  for (size_t i = 0; i < res.lines_num; i++)
-  {
+  for (size_t i = 0; i < res.lines_num; i++) {
     char *time_str;
     char *ident_str;
 
     /* First field is the time. */
     time_str = res.lines[i];
 
     /* Set `ident_str' to the beginning of the second field. */
     ident_str = time_str;
     while ((*ident_str != ' ') && (*ident_str != '\t') && (*ident_str != 0))
       ident_str++;
-    while ((*ident_str == ' ') || (*ident_str == '\t'))
-    {
+    while ((*ident_str == ' ') || (*ident_str == '\t')) {
       *ident_str = 0;
       ident_str++;
     }
 
-    if (*ident_str == 0)
-    {
-      lcc_set_errno (c, EILSEQ);
+    if (*ident_str == 0) {
+      lcc_set_errno(c, EILSEQ);
       status = -1;
       break;
     }
 
-    status = lcc_string_to_identifier (c, ident + i, ident_str);
+    status = lcc_string_to_identifier(c, ident + i, ident_str);
     if (status != 0)
       break;
   }
 
-  lcc_response_free (&res);
+  lcc_response_free(&res);
 
-  if (status != 0)
-  {
-    free (ident);
+  if (status != 0) {
+    free(ident);
     return (-1);
   }
 
   *ret_ident = ident;
   *ret_ident_num = ident_num;
 
   return (0);
 } /* }}} int lcc_listval */
 
-const char *lcc_strerror (lcc_connection_t *c) /* {{{ */
+const char *lcc_strerror(lcc_connection_t *c) /* {{{ */
 {
   if (c == NULL)
     return ("Invalid object");
   return (c->errbuf);
 } /* }}} const char *lcc_strerror */
 
-int lcc_identifier_to_string (lcc_connection_t *c, /* {{{ */
-    char *string, size_t string_size, const lcc_identifier_t *ident)
-{
-  if ((string == NULL) || (string_size < 6) || (ident == NULL))
-  {
-    lcc_set_errno (c, EINVAL);
+int lcc_identifier_to_string(lcc_connection_t *c, /* {{{ */
+                             char *string, size_t string_size,
+                             const lcc_identifier_t *ident) {
+  if ((string == NULL) || (string_size < 6) || (ident == NULL)) {
+    lcc_set_errno(c, EINVAL);
     return (-1);
   }
 
-  if (ident->plugin_instance[0] == 0)
-  {
+  if (ident->plugin_instance[0] == 0) {
     if (ident->type_instance[0] == 0)
-      snprintf (string, string_size, "%s/%s/%s",
-          ident->host,
-          ident->plugin,
-          ident->type);
+      snprintf(string, string_size, "%s/%s/%s", ident->host, ident->plugin,
+               ident->type);
     else
-      snprintf (string, string_size, "%s/%s/%s-%s",
-          ident->host,
-          ident->plugin,
-          ident->type,
-          ident->type_instance);
-  }
-  else
-  {
+      snprintf(string, string_size, "%s/%s/%s-%s", ident->host, ident->plugin,
+               ident->type, ident->type_instance);
+  } else {
     if (ident->type_instance[0] == 0)
-      snprintf (string, string_size, "%s/%s-%s/%s",
-          ident->host,
-          ident->plugin,
-          ident->plugin_instance,
-          ident->type);
+      snprintf(string, string_size, "%s/%s-%s/%s", ident->host, ident->plugin,
+               ident->plugin_instance, ident->type);
     else
-      snprintf (string, string_size, "%s/%s-%s/%s-%s",
-          ident->host,
-          ident->plugin,
-          ident->plugin_instance,
-          ident->type,
-          ident->type_instance);
+      snprintf(string, string_size, "%s/%s-%s/%s-%s", ident->host,
+               ident->plugin, ident->plugin_instance, ident->type,
+               ident->type_instance);
   }
 
   string[string_size - 1] = 0;
   return (0);
 } /* }}} int lcc_identifier_to_string */
 
-int lcc_string_to_identifier (lcc_connection_t *c, /* {{{ */
-    lcc_identifier_t *ident, const char *string)
-{
+int lcc_string_to_identifier(lcc_connection_t *c, /* {{{ */
+                             lcc_identifier_t *ident, const char *string) {
   char *string_copy;
   char *host;
   char *plugin;
   char *plugin_instance;
   char *type;
   char *type_instance;
 
-  string_copy = strdup (string);
-  if (string_copy == NULL)
-  {
-    lcc_set_errno (c, ENOMEM);
+  string_copy = strdup(string);
+  if (string_copy == NULL) {
+    lcc_set_errno(c, ENOMEM);
     return (-1);
   }
 
   host = string_copy;
-  plugin = strchr (host, '/');
-  if (plugin == NULL)
-  {
-    LCC_SET_ERRSTR (c, "Malformed identifier string: %s", string);
-    free (string_copy);
+  plugin = strchr(host, '/');
+  if (plugin == NULL) {
+    LCC_SET_ERRSTR(c, "Malformed identifier string: %s", string);
+    free(string_copy);
     return (-1);
   }
   *plugin = 0;
   plugin++;
 
-  type = strchr (plugin, '/');
-  if (type == NULL)
-  {
-    LCC_SET_ERRSTR (c, "Malformed identifier string: %s", string);
-    free (string_copy);
+  type = strchr(plugin, '/');
+  if (type == NULL) {
+    LCC_SET_ERRSTR(c, "Malformed identifier string: %s", string);
+    free(string_copy);
     return (-1);
   }
   *type = 0;
   type++;
 
-  plugin_instance = strchr (plugin, '-');
-  if (plugin_instance != NULL)
-  {
+  plugin_instance = strchr(plugin, '-');
+  if (plugin_instance != NULL) {
     *plugin_instance = 0;
     plugin_instance++;
   }
 
-  type_instance = strchr (type, '-');
-  if (type_instance != NULL)
-  {
+  type_instance = strchr(type, '-');
+  if (type_instance != NULL) {
     *type_instance = 0;
     type_instance++;
   }
 
-  memset (ident, 0, sizeof (*ident));
+  memset(ident, 0, sizeof(*ident));
 
-  SSTRCPY (ident->host, host);
-  SSTRCPY (ident->plugin, plugin);
+  SSTRCPY(ident->host, host);
+  SSTRCPY(ident->plugin, plugin);
   if (plugin_instance != NULL)
-    SSTRCPY (ident->plugin_instance, plugin_instance);
-  SSTRCPY (ident->type, type);
+    SSTRCPY(ident->plugin_instance, plugin_instance);
+  SSTRCPY(ident->type, type);
   if (type_instance != NULL)
-    SSTRCPY (ident->type_instance, type_instance);
+    SSTRCPY(ident->type_instance, type_instance);
 
-  free (string_copy);
+  free(string_copy);
   return (0);
 } /* }}} int lcc_string_to_identifier */
 
-int lcc_identifier_compare (const void *a, /* {{{ */
-    const void *b)
-{
+int lcc_identifier_compare(const void *a, /* {{{ */
+                           const void *b) {
   const lcc_identifier_t *i0 = a;
   const lcc_identifier_t *i1 = b;
   int status;
 
   if ((i0 == NULL) && (i1 == NULL))
     return (0);
   else if (i0 == NULL)
     return (-1);
   else if (i1 == NULL)
     return (1);
 
-#define CMP_FIELD(f) do {         \
-  status = strcmp (i0->f, i1->f); \
-  if (status != 0)                \
-    return (status);              \
-} while (0);
-
-    CMP_FIELD (host);
-    CMP_FIELD (plugin);
-    CMP_FIELD (plugin_instance);
-    CMP_FIELD (type);
-    CMP_FIELD (type_instance);
+#define CMP_FIELD(f)                                                           \
+  do {                                                                         \
+    status = strcmp(i0->f, i1->f);                                             \
+    if (status != 0)                                                           \
+      return (status);                                                         \
+  } while (0);
+
+  CMP_FIELD(host);
+  CMP_FIELD(plugin);
+  CMP_FIELD(plugin_instance);
+  CMP_FIELD(type);
+  CMP_FIELD(type_instance);
 
 #undef CMP_FIELD
 
-    return (0);
+  return (0);
 } /* }}} int lcc_identifier_compare */
 
-int lcc_sort_identifiers (lcc_connection_t *c, /* {{{ */
-    lcc_identifier_t *idents, size_t idents_num)
-{
-  if (idents == NULL)
-  {
-    lcc_set_errno (c, EINVAL);
+int lcc_sort_identifiers(lcc_connection_t *c, /* {{{ */
+                         lcc_identifier_t *idents, size_t idents_num) {
+  if (idents == NULL) {
+    lcc_set_errno(c, EINVAL);
     return (-1);
   }
 
-  qsort (idents, idents_num, sizeof (*idents),
-      lcc_identifier_compare);
+  qsort(idents, idents_num, sizeof(*idents), lcc_identifier_compare);
   return (0);
 } /* }}} int lcc_sort_identifiers */
 
 /* vim: set sw=2 sts=2 et fdm=marker : */
