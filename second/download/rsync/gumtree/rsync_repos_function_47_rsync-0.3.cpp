static void send_file_name(int f,struct file_list *flist,
			   int recurse,char *fname)
{
  struct file_struct *file;

  file = make_file(recurse,fname);

  if (!file) return;
  
  if (flist->count >= flist_malloced) {
    flist_malloced += 100;
    flist->files = (struct file_struct *)realloc(flist->files,
						 sizeof(flist->files[0])*
						 flist_malloced);
    if (!flist->files)
      out_of_memory("send_file_name");
  }

  flist->files[flist->count++] = *file;    

  send_file_entry(file,f);

  if (S_ISDIR(file->mode) && recurse) {
    char **last_exclude_list = local_exclude_list;
    send_directory(f,flist,file->name);
    local_exclude_list = last_exclude_list;
    return;
  }
}