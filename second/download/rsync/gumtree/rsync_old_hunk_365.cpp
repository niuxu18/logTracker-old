
  file = (struct file_struct *)malloc(sizeof(*file));
  if (!file) out_of_memory("receive_file_entry");
  bzero((char *)file,sizeof(*file));
  (*fptr) = file;

  strncpy(thisname,lastname,l1);
  read_buf(f,&thisname[l1],l2);
  thisname[l1+l2] = 0;

  strncpy(lastname,thisname,MAXPATHLEN-1);
  lastname[MAXPATHLEN-1] = 0;

  clean_fname(thisname);
