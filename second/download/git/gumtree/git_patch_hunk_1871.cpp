 			break;
 	}
 	free(last_branch);
 	return logs_found;
 }
 
-/* This function should make sure errno is meaningful on error */
+/*
+ * Locks a ref returning the lock on success and NULL on failure.
+ * On failure errno is set to something meaningful.
+ */
 static struct ref_lock *lock_ref_sha1_basic(const char *refname,
 					    const unsigned char *old_sha1,
+					    const struct string_list *skip,
 					    int flags, int *type_p)
 {
 	char *ref_file;
 	const char *orig_refname = refname;
 	struct ref_lock *lock;
 	int last_errno = 0;
 	int type, lflags;
 	int mustexist = (old_sha1 && !is_null_sha1(old_sha1));
+	int resolve_flags = 0;
 	int missing = 0;
 	int attempts_remaining = 3;
 
 	lock = xcalloc(1, sizeof(struct ref_lock));
 	lock->lock_fd = -1;
 
-	refname = resolve_ref_unsafe(refname, lock->old_sha1, mustexist, &type);
+	if (mustexist)
+		resolve_flags |= RESOLVE_REF_READING;
+	if (flags & REF_DELETING) {
+		resolve_flags |= RESOLVE_REF_ALLOW_BAD_NAME;
+		if (flags & REF_NODEREF)
+			resolve_flags |= RESOLVE_REF_NO_RECURSE;
+	}
+
+	refname = resolve_ref_unsafe(refname, resolve_flags,
+				     lock->old_sha1, &type);
 	if (!refname && errno == EISDIR) {
 		/* we are trying to lock foo but we used to
 		 * have foo/bar which now does not exist;
 		 * it is normal for the empty directory 'foo'
 		 * to remain.
 		 */
 		ref_file = git_path("%s", orig_refname);
 		if (remove_empty_directories(ref_file)) {
 			last_errno = errno;
 			error("there are still refs under '%s'", orig_refname);
 			goto error_return;
 		}
-		refname = resolve_ref_unsafe(orig_refname, lock->old_sha1, mustexist, &type);
+		refname = resolve_ref_unsafe(orig_refname, resolve_flags,
+					     lock->old_sha1, &type);
 	}
 	if (type_p)
 	    *type_p = type;
 	if (!refname) {
 		last_errno = errno;
 		error("unable to resolve reference %s: %s",
