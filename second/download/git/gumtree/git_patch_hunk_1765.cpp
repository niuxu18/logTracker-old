 	else
 		die(_("Invalid cleanup mode %s"), cleanup_arg);
 
 	if (annotate)
 		create_tag(object, tag, &buf, &opt, prev, object);
 
-	lock = lock_any_ref_for_update(ref.buf, prev, 0, NULL);
-	if (!lock)
-		die(_("%s: cannot lock the ref"), ref.buf);
-	if (write_ref_sha1(lock, object, NULL) < 0)
-		die(_("%s: cannot update the ref"), ref.buf);
+	transaction = ref_transaction_begin(&err);
+	if (!transaction ||
+	    ref_transaction_update(transaction, ref.buf, object, prev,
+				   0, 1, NULL, &err) ||
+	    ref_transaction_commit(transaction, &err))
+		die("%s", err.buf);
+	ref_transaction_free(transaction);
 	if (force && !is_null_sha1(prev) && hashcmp(prev, object))
 		printf(_("Updated tag '%s' (was %s)\n"), tag, find_unique_abbrev(prev, DEFAULT_ABBREV));
 
+	strbuf_release(&err);
 	strbuf_release(&buf);
 	strbuf_release(&ref);
 	return 0;
 }
