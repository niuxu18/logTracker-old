 			if (first_parent)
 				break;
 		}
 	}
 
 	if (!match_cnt) {
-		const unsigned char *sha1 = cmit->object.sha1;
+		struct object_id *oid = &cmit->object.oid;
 		if (always) {
-			printf("%s", find_unique_abbrev(sha1, abbrev));
+			printf("%s", find_unique_abbrev(oid->hash, abbrev));
 			if (dirty)
 				printf("%s", dirty);
 			printf("\n");
 			return;
 		}
 		if (unannotated_cnt)
 			die(_("No annotated tags can describe '%s'.\n"
 			    "However, there were unannotated tags: try --tags."),
-			    sha1_to_hex(sha1));
+			    oid_to_hex(oid));
 		else
 			die(_("No tags can describe '%s'.\n"
 			    "Try --always, or create some tags."),
-			    sha1_to_hex(sha1));
+			    oid_to_hex(oid));
 	}
 
 	qsort(all_matches, match_cnt, sizeof(all_matches[0]), compare_pt);
 
 	if (gave_up_on) {
 		commit_list_insert_by_date(gave_up_on, &list);
