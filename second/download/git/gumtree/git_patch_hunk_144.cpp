 				attr |= (plain_attr & BACKGROUND_ALL);
 				break;
 			default:
 				/* Unsupported code */
 				break;
 			}
-			str++;
-		} while (*(str-1) == ';');
-
+		}
 		set_console_attr();
 		break;
 	case 'K':
 		erase_in_line();
 		break;
 	default:
 		/* Unsupported code */
 		break;
 	}
-
-	return func + 1;
 }
 
-static int ansi_emulate(const char *str, FILE *stream)
-{
-	int rv = 0;
-	const char *pos = str;
+enum {
+	TEXT = 0, ESCAPE = 033, BRACKET = '['
+};
 
-	while (*pos) {
-		pos = strstr(str, "\033[");
-		if (pos) {
-			size_t len = pos - str;
+static DWORD WINAPI console_thread(LPVOID unused)
+{
+	unsigned char buffer[BUFFER_SIZE];
+	DWORD bytes;
+	int start, end = 0, c, parampos = 0, state = TEXT;
+	int params[MAX_PARAMS];
+
+	while (1) {
+		/* read next chunk of bytes from the pipe */
+		if (!ReadFile(hread, buffer + end, BUFFER_SIZE - end, &bytes,
+				NULL)) {
+			/* exit if pipe has been closed or disconnected */
+			if (GetLastError() == ERROR_PIPE_NOT_CONNECTED ||
+					GetLastError() == ERROR_BROKEN_PIPE)
+				break;
+			/* ignore other errors */
+			continue;
+		}
 
-			if (len) {
-				size_t out_len = fwrite(str, 1, len, stream);
-				rv += out_len;
-				if (out_len < len)
-					return rv;
+		/* scan the bytes and handle ANSI control codes */
+		bytes += end;
+		start = end = 0;
+		while (end < bytes) {
+			c = buffer[end++];
+			switch (state) {
+			case TEXT:
+				if (c == ESCAPE) {
+					/* print text seen so far */
+					if (end - 1 > start)
+						write_console(buffer + start,
+							end - 1 - start);
+
+					/* then start parsing escape sequence */
+					start = end - 1;
+					memset(params, 0, sizeof(params));
+					parampos = 0;
+					state = ESCAPE;
+				}
+				break;
+
+			case ESCAPE:
+				/* continue if "\033[", otherwise bail out */
+				state = (c == BRACKET) ? BRACKET : TEXT;
+				break;
+
+			case BRACKET:
+				/* parse [0-9;]* into array of parameters */
+				if (c >= '0' && c <= '9') {
+					params[parampos] *= 10;
+					params[parampos] += c - '0';
+				} else if (c == ';') {
+					/*
+					 * next parameter, bail out if out of
+					 * bounds
+					 */
+					parampos++;
+					if (parampos >= MAX_PARAMS)
+						state = TEXT;
+				} else {
+					/*
+					 * end of escape sequence, change
+					 * console attributes
+					 */
+					set_attr(c, params, parampos + 1);
+					start = end;
+					state = TEXT;
+				}
+				break;
 			}
+		}
 
-			str = pos + 2;
-			rv += 2;
-
-			fflush(stream);
+		/* print remaining text unless parsing an escape sequence */
+		if (state == TEXT && end > start) {
+			/* check for incomplete UTF-8 sequences and fix end */
+			if (buffer[end - 1] >= 0x80) {
+				if (buffer[end -1] >= 0xc0)
+					end--;
+				else if (end - 1 > start &&
+						buffer[end - 2] >= 0xe0)
+					end -= 2;
+				else if (end - 2 > start &&
+						buffer[end - 3] >= 0xf0)
+					end -= 3;
+			}
 
-			pos = set_attr(str);
-			rv += pos - str;
-			str = pos;
+			/* print remaining complete UTF-8 sequences */
+			if (end > start)
+				write_console(buffer + start, end - start);
+
+			/* move remaining bytes to the front */
+			if (end < bytes)
+				memmove(buffer, buffer + end, bytes - end);
+			end = bytes - end;
 		} else {
-			rv += strlen(str);
-			fputs(str, stream);
-			return rv;
+			/* all data has been consumed, mark buffer empty */
+			end = 0;
 		}
 	}
-	return rv;
-}
 
-int winansi_fputs(const char *str, FILE *stream)
-{
-	int rv;
-
-	if (!isatty(fileno(stream)))
-		return fputs(str, stream);
+	/* check if the console font supports unicode */
+	warn_if_raster_font();
 
-	init();
-
-	if (!console)
-		return fputs(str, stream);
+	CloseHandle(hread);
+	return 0;
+}
 
-	rv = ansi_emulate(str, stream);
+static void winansi_exit(void)
+{
+	/* flush all streams */
+	_flushall();
 
-	if (rv >= 0)
-		return 0;
-	else
-		return EOF;
+	/* signal console thread to exit */
+	FlushFileBuffers(hwrite);
+	DisconnectNamedPipe(hwrite);
+
+	/* wait for console thread to copy remaining data */
+	WaitForSingleObject(hthread, INFINITE);
+
+	/* cleanup handles... */
+	CloseHandle(hwrite);
+	CloseHandle(hthread);
 }
 
-static int winansi_vfprintf(FILE *stream, const char *format, va_list list)
+static void die_lasterr(const char *fmt, ...)
 {
-	int len, rv;
-	char small_buf[256];
-	char *buf = small_buf;
-	va_list cp;
+	va_list params;
+	va_start(params, fmt);
+	errno = err_win_to_posix(GetLastError());
+	die_errno(fmt, params);
+	va_end(params);
+}
 
-	if (!isatty(fileno(stream)))
-		goto abort;
+static HANDLE duplicate_handle(HANDLE hnd)
+{
+	HANDLE hresult, hproc = GetCurrentProcess();
+	if (!DuplicateHandle(hproc, hnd, hproc, &hresult, 0, TRUE,
+			DUPLICATE_SAME_ACCESS))
+		die_lasterr("DuplicateHandle(%li) failed", (long) hnd);
+	return hresult;
+}
 
-	init();
 
-	if (!console)
-		goto abort;
+/*
+ * Make MSVCRT's internal file descriptor control structure accessible
+ * so that we can tweak OS handles and flags directly (we need MSVCRT
+ * to treat our pipe handle as if it were a console).
+ *
+ * We assume that the ioinfo structure (exposed by MSVCRT.dll via
+ * __pioinfo) starts with the OS handle and the flags. The exact size
+ * varies between MSVCRT versions, so we try different sizes until
+ * toggling the FDEV bit of _pioinfo(1)->osflags is reflected in
+ * isatty(1).
+ */
+typedef struct {
+	HANDLE osfhnd;
+	char osflags;
+} ioinfo;
 
-	va_copy(cp, list);
-	len = vsnprintf(small_buf, sizeof(small_buf), format, cp);
-	va_end(cp);
-
-	if (len > sizeof(small_buf) - 1) {
-		buf = malloc(len + 1);
-		if (!buf)
-			goto abort;
+extern __declspec(dllimport) ioinfo *__pioinfo[];
 
-		len = vsnprintf(buf, len + 1, format, list);
-	}
+static size_t sizeof_ioinfo = 0;
 
-	rv = ansi_emulate(buf, stream);
+#define IOINFO_L2E 5
+#define IOINFO_ARRAY_ELTS (1 << IOINFO_L2E)
 
-	if (buf != small_buf)
-		free(buf);
-	return rv;
+#define FDEV  0x40
 
-abort:
-	rv = vfprintf(stream, format, list);
-	return rv;
+static inline ioinfo* _pioinfo(int fd)
+{
+	return (ioinfo*)((char*)__pioinfo[fd >> IOINFO_L2E] +
+			(fd & (IOINFO_ARRAY_ELTS - 1)) * sizeof_ioinfo);
 }
 
-int winansi_fprintf(FILE *stream, const char *format, ...)
+static int init_sizeof_ioinfo()
 {
-	va_list list;
-	int rv;
+	int istty, wastty;
+	/* don't init twice */
+	if (sizeof_ioinfo)
+		return sizeof_ioinfo >= 256;
+
+	sizeof_ioinfo = sizeof(ioinfo);
+	wastty = isatty(1);
+	while (sizeof_ioinfo < 256) {
+		/* toggle FDEV flag, check isatty, then toggle back */
+		_pioinfo(1)->osflags ^= FDEV;
+		istty = isatty(1);
+		_pioinfo(1)->osflags ^= FDEV;
+		/* return if we found the correct size */
+		if (istty != wastty)
+			return 0;
+		sizeof_ioinfo += sizeof(void*);
+	}
+	error("Tweaking file descriptors doesn't work with this MSVCRT.dll");
+	return 1;
+}
 
-	va_start(list, format);
-	rv = winansi_vfprintf(stream, format, list);
-	va_end(list);
+static HANDLE swap_osfhnd(int fd, HANDLE new_handle)
+{
+	ioinfo *pioinfo;
+	HANDLE old_handle;
 
-	return rv;
+	/* init ioinfo size if we haven't done so */
+	if (init_sizeof_ioinfo())
+		return INVALID_HANDLE_VALUE;
+
+	/* get ioinfo pointer and change the handles */
+	pioinfo = _pioinfo(fd);
+	old_handle = pioinfo->osfhnd;
+	pioinfo->osfhnd = new_handle;
+	return old_handle;
 }
 
-int winansi_printf(const char *format, ...)
+void winansi_init(void)
 {
-	va_list list;
-	int rv;
+	int con1, con2;
+	char name[32];
+
+	/* check if either stdout or stderr is a console output screen buffer */
+	con1 = is_console(1);
+	con2 = is_console(2);
+	if (!con1 && !con2)
+		return;
 
-	va_start(list, format);
-	rv = winansi_vfprintf(stdout, format, list);
-	va_end(list);
+	/* create a named pipe to communicate with the console thread */
+	sprintf(name, "\\\\.\\pipe\\winansi%lu", GetCurrentProcessId());
+	hwrite = CreateNamedPipe(name, PIPE_ACCESS_OUTBOUND,
+		PIPE_TYPE_BYTE | PIPE_WAIT, 1, BUFFER_SIZE, 0, 0, NULL);
+	if (hwrite == INVALID_HANDLE_VALUE)
+		die_lasterr("CreateNamedPipe failed");
+
+	hread = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
+	if (hread == INVALID_HANDLE_VALUE)
+		die_lasterr("CreateFile for named pipe failed");
+
+	/* start console spool thread on the pipe's read end */
+	hthread = CreateThread(NULL, 0, console_thread, NULL, 0, NULL);
+	if (hthread == INVALID_HANDLE_VALUE)
+		die_lasterr("CreateThread(console_thread) failed");
+
+	/* schedule cleanup routine */
+	if (atexit(winansi_exit))
+		die_errno("atexit(winansi_exit) failed");
+
+	/* redirect stdout / stderr to the pipe */
+	if (con1)
+		hconsole1 = swap_osfhnd(1, duplicate_handle(hwrite));
+	if (con2)
+		hconsole2 = swap_osfhnd(2, duplicate_handle(hwrite));
+}
 
-	return rv;
+/*
+ * Returns the real console handle if stdout / stderr is a pipe redirecting
+ * to the console. Allows spawn / exec to pass the console to the next process.
+ */
+HANDLE winansi_get_osfhandle(int fd)
+{
+	HANDLE hnd = (HANDLE) _get_osfhandle(fd);
+	if ((fd == 1 || fd == 2) && isatty(fd)
+	    && GetFileType(hnd) == FILE_TYPE_PIPE)
+		return (fd == 1) ? hconsole1 : hconsole2;
+	return hnd;
 }
