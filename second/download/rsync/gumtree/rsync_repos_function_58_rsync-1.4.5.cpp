static void receive_data(int f_in,char *buf,int fd,char *fname)
{
  int i,n,remainder,len,count;
  off_t offset = 0;
  off_t offset2;

  count = read_int(f_in);
  n = read_int(f_in);
  remainder = read_int(f_in);

  for (i=read_int(f_in); i != 0; i=read_int(f_in)) {
    if (i > 0) {
      if (verbose > 3)
	fprintf(FERROR,"data recv %d at %d\n",i,(int)offset);

      if (read_write(f_in,fd,i) != i) {
	fprintf(FERROR,"write failed on %s : %s\n",fname,strerror(errno));
	exit_cleanup(1);
      }
      offset += i;
    } else {
      i = -(i+1);
      offset2 = i*n;
      len = n;
      if (i == count-1 && remainder != 0)
	len = remainder;

      if (verbose > 3)
	fprintf(FERROR,"chunk[%d] of size %d at %d offset=%d\n",
		i,len,(int)offset2,(int)offset);

      if (write_sparse(fd,map_ptr(buf,offset2,len),len) != len) {
	fprintf(FERROR,"write failed on %s : %s\n",fname,strerror(errno));
	exit_cleanup(1);
      }
      offset += len;
    }
  }

  if (offset > 0 && sparse_end(fd) != 0) {
    fprintf(FERROR,"write failed on %s : %s\n",fname,strerror(errno));
    exit_cleanup(1);
  }
}