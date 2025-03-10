    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
 
 #include "rsync.h"
 
 extern int verbose;
-extern int csum_length;
-extern struct stats stats;
-extern int io_error;
 extern int dry_run;
 extern int am_server;
 extern int am_daemon;
+extern int log_before_transfer;
+extern int log_format_has_i;
+extern int daemon_log_format_has_i;
+extern int csum_length;
+extern int io_error;
 extern int protocol_version;
+extern int remove_sent_files;
+extern int updating_basis_file;
 extern int make_backups;
+extern int do_progress;
+extern int inplace;
 extern struct stats stats;
+extern struct file_list *the_file_list;
+extern char *log_format;
 
 
 /**
  * @file
  *
  * The sender gets checksums from the generator, calculates deltas,
  * and transmits them to the receiver.  The sender process runs on the
  * machine holding the source files.
  **/
-void read_sum_head(int f, struct sum_struct *sum)
-{
-	sum->count = read_int(f);
-	sum->blength = read_int(f);
-	if (protocol_version < 27) {
-		sum->s2length = csum_length;
-	} else {
-		sum->s2length = read_int(f);
-		if (sum->s2length > MD4_SUM_LENGTH) {
-			rprintf(FERROR, "Invalid checksum length %ld\n",
-			    (long)sum->s2length);
-			exit_cleanup(RERR_PROTOCOL);
-		}
-	}
-	sum->remainder = read_int(f);
-}
 
 /**
  * Receive the checksums for a buffer
  **/
 static struct sum_struct *receive_sums(int f)
 {
 	struct sum_struct *s;
-	int i;
+	int32 i;
 	OFF_T offset = 0;
 
 	if (!(s = new(struct sum_struct)))
 		out_of_memory("receive_sums");
 
 	read_sum_head(f, s);
 
 	s->sums = NULL;
 
 	if (verbose > 3) {
-		rprintf(FINFO, "count=%ld n=%u rem=%u\n",
-			(long)s->count, s->blength, s->remainder);
+		rprintf(FINFO, "count=%.0f n=%ld rem=%ld\n",
+			(double)s->count, (long)s->blength, (long)s->remainder);
 	}
 
 	if (s->count == 0)
 		return(s);
 
 	if (!(s->sums = new_array(struct sum_buf, s->count)))
 		out_of_memory("receive_sums");
 
-	for (i = 0; i < (int)s->count; i++) {
+	for (i = 0; i < s->count; i++) {
 		s->sums[i].sum1 = read_int(f);
 		read_buf(f, s->sums[i].sum2, s->s2length);
 
 		s->sums[i].offset = offset;
 		s->sums[i].flags = 0;
 
-		if (i == (int)s->count-1 && s->remainder != 0)
+		if (i == s->count-1 && s->remainder != 0)
 			s->sums[i].len = s->remainder;
 		else
 			s->sums[i].len = s->blength;
 		offset += s->sums[i].len;
 
 		if (verbose > 3) {
