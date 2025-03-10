static int receive_data(int f_in,struct map_struct *buf,int fd,char *fname)
{
  int i,n,remainder,len,count;
  OFF_T offset = 0;
  OFF_T offset2;
  char *data;
  static char file_sum1[MD4_SUM_LENGTH];
  static char file_sum2[MD4_SUM_LENGTH];
  char *map=NULL;

  count = read_int(f_in);
  n = read_int(f_in);
  remainder = read_int(f_in);

  sum_init();

  for (i=recv_token(f_in,&data); i != 0; i=recv_token(f_in,&data)) {
    if (i > 0) {
      if (verbose > 3)
	rprintf(FINFO,"data recv %d at %d\n",i,(int)offset);

      stats.literal_data += i;
      sum_update(data,i);

      if (fd != -1 && write_file(fd,data,i) != i) {
	rprintf(FERROR,"write failed on %s : %s\n",fname,strerror(errno));
	exit_cleanup(1);
      }
      offset += i;
    } else {
      i = -(i+1);
      offset2 = i*n;
      len = n;
      if (i == count-1 && remainder != 0)
	len = remainder;

      stats.matched_data += len;

      if (verbose > 3)
	rprintf(FINFO,"chunk[%d] of size %d at %d offset=%d\n",
		i,len,(int)offset2,(int)offset);

      map = map_ptr(buf,offset2,len);

      see_token(map, len);
      sum_update(map,len);

      if (fd != -1 && write_file(fd,map,len) != len) {
	rprintf(FERROR,"write failed on %s : %s\n",fname,strerror(errno));
	exit_cleanup(1);
      }
      offset += len;
    }
  }

  if (fd != -1 && offset > 0 && sparse_end(fd) != 0) {
    rprintf(FERROR,"write failed on %s : %s\n",fname,strerror(errno));
    exit_cleanup(1);
  }

  sum_end(file_sum1);

  if (remote_version >= 14) {
    read_buf(f_in,file_sum2,MD4_SUM_LENGTH);
    if (verbose > 2)
      rprintf(FINFO,"got file_sum\n");
    if (fd != -1 && memcmp(file_sum1,file_sum2,MD4_SUM_LENGTH) != 0)
      return 0;
  }
  return 1;
}