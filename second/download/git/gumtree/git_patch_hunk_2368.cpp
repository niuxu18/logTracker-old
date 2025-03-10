 
 	if (log && rename_tmp_log(newrefname))
 		goto rollback;
 
 	logmoved = log;
 
-	lock = lock_ref_sha1_basic(newrefname, NULL, NULL, 0, NULL);
+	lock = lock_ref_sha1_basic(newrefname, NULL, NULL, NULL, 0, NULL, &err);
 	if (!lock) {
-		error("unable to lock %s for update", newrefname);
+		error("unable to rename '%s' to '%s': %s", oldrefname, newrefname, err.buf);
+		strbuf_release(&err);
 		goto rollback;
 	}
 	hashcpy(lock->old_sha1, orig_sha1);
-	if (write_ref_sha1(lock, orig_sha1, logmsg)) {
+
+	if (write_ref_to_lockfile(lock, orig_sha1) ||
+	    commit_ref_update(lock, orig_sha1, logmsg)) {
 		error("unable to write current sha1 into %s", newrefname);
 		goto rollback;
 	}
 
 	return 0;
 
  rollback:
-	lock = lock_ref_sha1_basic(oldrefname, NULL, NULL, 0, NULL);
+	lock = lock_ref_sha1_basic(oldrefname, NULL, NULL, NULL, 0, NULL, &err);
 	if (!lock) {
-		error("unable to lock %s for rollback", oldrefname);
+		error("unable to lock %s for rollback: %s", oldrefname, err.buf);
+		strbuf_release(&err);
 		goto rollbacklog;
 	}
 
 	flag = log_all_ref_updates;
 	log_all_ref_updates = 0;
-	if (write_ref_sha1(lock, orig_sha1, NULL))
+	if (write_ref_to_lockfile(lock, orig_sha1) ||
+	    commit_ref_update(lock, orig_sha1, NULL))
 		error("unable to write current sha1 into %s", oldrefname);
 	log_all_ref_updates = flag;
 
  rollbacklog:
 	if (logmoved && rename(git_path("logs/%s", newrefname), git_path("logs/%s", oldrefname)))
 		error("unable to restore logfile %s from %s: %s",
