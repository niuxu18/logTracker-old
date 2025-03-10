 	if (n->tag)
 		printf("%s", n->tag->tag);
 	else
 		printf("%s", n->path);
 }
 
-static void show_suffix(int depth, const unsigned char *sha1)
+static void show_suffix(int depth, const struct object_id *oid)
 {
-	printf("-%d-g%s", depth, find_unique_abbrev(sha1, abbrev));
+	printf("-%d-g%s", depth, find_unique_abbrev(oid->hash, abbrev));
 }
 
 static void describe(const char *arg, int last_one)
 {
-	unsigned char sha1[20];
+	struct object_id oid;
 	struct commit *cmit, *gave_up_on = NULL;
 	struct commit_list *list;
 	struct commit_name *n;
 	struct possible_tag all_matches[MAX_TAGS];
 	unsigned int match_cnt = 0, annotated_cnt = 0, cur_match;
 	unsigned long seen_commits = 0;
 	unsigned int unannotated_cnt = 0;
 
-	if (get_sha1(arg, sha1))
+	if (get_oid(arg, &oid))
 		die(_("Not a valid object name %s"), arg);
-	cmit = lookup_commit_reference(sha1);
+	cmit = lookup_commit_reference(oid.hash);
 	if (!cmit)
 		die(_("%s is not a valid '%s' object"), arg, commit_type);
 
-	n = find_commit_name(cmit->object.oid.hash);
+	n = find_commit_name(&cmit->object.oid);
 	if (n && (tags || all || n->prio == 2)) {
 		/*
 		 * Exact match to an existing ref.
 		 */
 		display_name(n);
 		if (longformat)
-			show_suffix(0, n->tag ? n->tag->tagged->oid.hash : sha1);
-		if (dirty)
-			printf("%s", dirty);
+			show_suffix(0, n->tag ? &n->tag->tagged->oid : &oid);
+		if (suffix)
+			printf("%s", suffix);
 		printf("\n");
 		return;
 	}
 
 	if (!max_candidates)
 		die(_("no tag exactly matches '%s'"), oid_to_hex(&cmit->object.oid));
