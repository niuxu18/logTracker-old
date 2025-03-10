 			continue;
 		}
 
 		free(name);
 
 		name = mkpathdup(fmt, bname.buf);
-		target = resolve_ref_unsafe(name, sha1, 0, &flags);
-		if (!target ||
-		    (!(flags & REF_ISSYMREF) && is_null_sha1(sha1))) {
+		target = resolve_ref_unsafe(name,
+					    RESOLVE_REF_READING
+					    | RESOLVE_REF_NO_RECURSE
+					    | RESOLVE_REF_ALLOW_BAD_NAME,
+					    sha1, &flags);
+		if (!target) {
 			error(remote_branch
 			      ? _("remote branch '%s' not found.")
 			      : _("branch '%s' not found."), bname.buf);
 			ret = 1;
 			continue;
 		}
 
-		if (!(flags & REF_ISSYMREF) &&
+		if (!(flags & (REF_ISSYMREF|REF_ISBROKEN)) &&
 		    check_branch_commit(bname.buf, name, sha1, head_rev, kinds,
 					force)) {
 			ret = 1;
 			continue;
 		}
 
