static int is_console(int fd)
{
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	DWORD mode;
	HANDLE hcon;

	static int initialized = 0;

	/* get OS handle of the file descriptor */
	hcon = (HANDLE) _get_osfhandle(fd);
	if (hcon == INVALID_HANDLE_VALUE)
		return 0;

	/* check if its a device (i.e. console, printer, serial port) */
	if (GetFileType(hcon) != FILE_TYPE_CHAR)
		return 0;

	/* check if its a handle to a console output screen buffer */
	if (!fd) {
		if (!GetConsoleMode(hcon, &mode))
			return 0;
		/*
		 * This code path is only reached if there is no console
		 * attached to stdout/stderr, i.e. we will not need to output
		 * any text to any console, therefore we might just as well
		 * use black as foreground color.
		 */
		sbi.wAttributes = 0;
	} else if (!GetConsoleScreenBufferInfo(hcon, &sbi))
		return 0;

	if (fd >= 0 && fd <= 2)
		fd_is_interactive[fd] |= FD_CONSOLE;

	/* initialize attributes */
	if (!initialized) {
		console = hcon;
		attr = plain_attr = sbi.wAttributes;
		negative = 0;
		initialized = 1;
	}

	return 1;
}