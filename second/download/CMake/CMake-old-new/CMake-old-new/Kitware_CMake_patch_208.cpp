@@ -6225,7 +6225,7 @@ isoent_gen_joliet_identifier(struct archive_write *a, struct isoent *isoent,
 	unsigned char *p;
 	size_t l;
 	int r;
-	int ffmax, parent_len;
+	size_t ffmax, parent_len;
 	static const struct archive_rb_tree_ops rb_ops = {
 		isoent_cmp_node_joliet, isoent_cmp_key_joliet
 	};
@@ -6239,7 +6239,7 @@ isoent_gen_joliet_identifier(struct archive_write *a, struct isoent *isoent,
 	else
 		ffmax = 128;
 
-	r = idr_start(a, idr, isoent->children.cnt, ffmax, 6, 2, &rb_ops);
+	r = idr_start(a, idr, isoent->children.cnt, (int)ffmax, 6, 2, &rb_ops);
 	if (r < 0)
 		return (r);
 
@@ -6252,7 +6252,7 @@ isoent_gen_joliet_identifier(struct archive_write *a, struct isoent *isoent,
 		int ext_off, noff, weight;
 		size_t lt;
 
-		if ((int)(l = np->file->basename_utf16.length) > ffmax)
+		if ((l = np->file->basename_utf16.length) > ffmax)
 			l = ffmax;
 
 		p = malloc((l+1)*2);
@@ -6285,7 +6285,7 @@ isoent_gen_joliet_identifier(struct archive_write *a, struct isoent *isoent,
 		/*
 		 * Get a length of MBS of a full-pathname.
 		 */
-		if ((int)np->file->basename_utf16.length > ffmax) {
+		if (np->file->basename_utf16.length > ffmax) {
 			if (archive_strncpy_l(&iso9660->mbs,
 			    (const char *)np->identifier, l,
 				iso9660->sconv_from_utf16be) != 0 &&
@@ -6302,7 +6302,9 @@ isoent_gen_joliet_identifier(struct archive_write *a, struct isoent *isoent,
 
 		/* If a length of full-pathname is longer than 240 bytes,
 		 * it violates Joliet extensions regulation. */
-		if (parent_len + np->mb_len > 240) {
+		if (parent_len > 240
+		    || np->mb_len > 240
+		    || parent_len + np->mb_len > 240) {
 			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
 			    "The regulation of Joliet extensions;"
 			    " A length of a full-pathname of `%s' is "
@@ -6314,11 +6316,11 @@ isoent_gen_joliet_identifier(struct archive_write *a, struct isoent *isoent,
 
 		/* Make an offset of the number which is used to be set
 		 * hexadecimal number to avoid duplicate identifier. */
-		if ((int)l == ffmax)
+		if (l == ffmax)
 			noff = ext_off - 6;
-		else if ((int)l == ffmax-2)
+		else if (l == ffmax-2)
 			noff = ext_off - 4;
-		else if ((int)l == ffmax-4)
+		else if (l == ffmax-4)
 			noff = ext_off - 2;
 		else
 			noff = ext_off;