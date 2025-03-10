void recv_uid_list(int f, struct file_list *flist);
int num_waiting(int fd);
struct map_struct *map_file(int fd,off_t len);
char *map_ptr(struct map_struct *map,off_t offset,int len);
void unmap_file(struct map_struct *map);
int piped_child(char **command,int *f_in,int *f_out);
void out_of_memory(char *str);
int set_modtime(char *fname,time_t modtime);
int set_blocking(int fd, int set);
int create_directory_path(char *fname);
int full_write(int desc, char *ptr, int len);
int safe_read(int desc, char *ptr, int len);
int copy_file(char *source, char *dest, mode_t mode);
