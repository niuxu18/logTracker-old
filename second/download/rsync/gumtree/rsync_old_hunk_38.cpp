	if (!am_server && verbose)
	  printf("%s\n",fname);
	write_int(f_out,i);
	continue;
      }

      fd = open(fname,O_RDONLY);
      if (fd == -1) {
	fprintf(stderr,"send_files failed to open %s: %s\n",
		fname,strerror(errno));
	return -1;
      }
  
      s = receive_sums(f_in);
      if (!s) 
	return -1;

      /* map the local file */
      if (fstat(fd,&st) != 0) 
	return -1;
      
      if (st.st_size > 0) {
	buf = map_file(fd,st.st_size);
