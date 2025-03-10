void
set_nonblocking(int fd)
{
    int val;

    if ((val = fcntl(fd, F_GETFL, 0)) == -1)
	return;
    if (!(val & NONBLOCK_FLAG)) {
	val |= NONBLOCK_FLAG;
	fcntl(fd, F_SETFL, val);
    }
}