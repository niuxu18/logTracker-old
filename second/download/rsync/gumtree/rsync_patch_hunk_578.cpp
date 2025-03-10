 char *f_name(struct file_struct *f);
 void recv_generator(char *fname,struct file_list *flist,int i,int f_out);
 void generate_files(int f,struct file_list *flist,char *local_name,int f_recv);
 void init_hard_links(struct file_list *flist);
 int check_hard_link(struct file_struct *file);
 void do_hard_links(struct file_list *flist);
-void setup_readbuffer(int f_in);
 void io_set_error_fd(int fd);
 int32 read_int(int f);
 int64 read_longint(int f);
 void read_buf(int f,char *buf,int len);
 void read_sbuf(int f,char *buf,int len);
 unsigned char read_byte(int f);
 void io_start_buffering(int fd);
 void io_flush(void);
 void io_end_buffering(int fd);
+void io_shutdown(void);
 void write_int(int f,int32 x);
 void write_longint(int f, int64 x);
 void write_buf(int f,char *buf,int len);
 void write_byte(int f,unsigned char c);
 int read_line(int f, char *buf, int maxlen);
 void io_printf(int fd, const char *format, ...);
 void io_start_multiplex_out(int fd);
 void io_start_multiplex_in(int fd);
 int io_multiplex_write(enum logcode code, char *buf, int len);
 int io_error_write(int f, enum logcode code, char *buf, int len);
 void io_multiplexing_close(void);
-void io_close_input(int fd);
 char *lp_motd_file(void);
 char *lp_log_file(void);
 char *lp_pid_file(void);
 char *lp_socket_options(void);
 int lp_syslog_facility(void);
 char *lp_name(int );
