			read_buffer = (char *)realloc(read_buffer,read_buffer_size);
		if (!read_buffer) out_of_memory("read check");      
		read_buffer_p = read_buffer;      
	}

	n = read_unbuffered(f,read_buffer+read_buffer_len,n);
	if (n > 0) {
		read_buffer_len += n;
	}
}

static int readfd(int fd,char *buffer,int N)
{
	int  ret;
	int total=0;  
	struct timeval tv;
	
	if (read_buffer_len < N)
		read_check(buffer_f_in);
	
	while (total < N) {
		if (read_buffer_len > 0 && buffer_f_in == fd) {
			ret = MIN(read_buffer_len,N-total);
			memcpy(buffer+total,read_buffer_p,ret);
			read_buffer_p += ret;
