struct file_list *send_file_list(int f,int recurse,int argc,char *argv[])
{
  int i,l;
  struct stat st;
  char *p,*dir;
  char dbuf[MAXPATHLEN];
  struct file_list *flist;

  if (verbose && recurse) {
    fprintf(FINFO,"building file list ... ");
    fflush(FINFO);
  }

  flist = (struct file_list *)malloc(sizeof(flist[0]));
  if (!flist) out_of_memory("send_file_list");

  flist->count=0;
  flist->malloced = 100;
  flist->files = (struct file_struct *)malloc(sizeof(flist->files[0])*
					      flist->malloced);
  if (!flist->files) out_of_memory("send_file_list");

  for (i=0;i<argc;i++) {
    char fname2[MAXPATHLEN];
    char *fname = fname2;

    strcpy(fname,argv[i]);

    l = strlen(fname);
    if (l != 1 && fname[l-1] == '/') {
      strcat(fname,".");
    }

    if (lstat(fname,&st) != 0) {
      fprintf(FERROR,"%s : %s\n",fname,strerror(errno));
      continue;
    }

    if (S_ISDIR(st.st_mode) && !recurse) {
      fprintf(FERROR,"skipping directory %s\n",fname);
      continue;
    }

    dir = NULL;
    p = strrchr(fname,'/');
    if (p) {
      *p = 0;
      dir = fname;
      fname = p+1;      
    }
    if (!*fname)
      fname = ".";

    if (dir && *dir) {
      if (getcwd(dbuf,MAXPATHLEN-1) == NULL) {
	fprintf(FERROR,"getwd : %s\n",strerror(errno));
	exit_cleanup(1);
      }
      if (chdir(dir) != 0) {
	fprintf(FERROR,"chdir %s : %s\n",dir,strerror(errno));
	continue;
      }
      flist_dir = dir;
      if (one_file_system)
	set_filesystem(fname);
      send_file_name(f,flist,recurse,fname);
      flist_dir = NULL;
      if (chdir(dbuf) != 0) {
	fprintf(FERROR,"chdir %s : %s\n",dbuf,strerror(errno));
	exit_cleanup(1);
      }
      continue;
    }

    if (one_file_system)
      set_filesystem(fname);
    send_file_name(f,flist,recurse,fname);
  }

  if (f != -1) {
    send_file_entry(NULL,f);
    write_flush(f);
  }

  if (verbose && recurse)
    fprintf(FINFO,"done\n");

  clean_flist(flist);

  return flist;
}