  if (verbose > 2)
    if (i != -1)
      fprintf(stderr,"match at %d last_match=%d j=%d len=%d n=%d\n",
	      (int)offset,(int)last_match,i,(int)s->sums[i].len,n);

  if (n > 0) {
    read_check(check_f_in);
    write_int(f,n);
    write_buf(f,buf+last_match,n);
    data_transfer += n;
  }
  write_int(f,-(i+1));
  if (i != -1)
