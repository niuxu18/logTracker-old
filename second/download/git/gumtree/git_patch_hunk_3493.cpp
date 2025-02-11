 	if (bisect_list)
 		revs.limited = 1;
 
 	if (use_bitmap_index && !revs.prune) {
 		if (revs.count && !revs.left_right && !revs.cherry_mark) {
 			uint32_t commit_count;
+			int max_count = revs.max_count;
 			if (!prepare_bitmap_walk(&revs)) {
 				count_bitmap_commit_list(&commit_count, NULL, NULL, NULL);
+				if (max_count >= 0 && max_count < commit_count)
+					commit_count = max_count;
 				printf("%d\n", commit_count);
 				return 0;
 			}
-		} else if (revs.tag_objects && revs.tree_objects && revs.blob_objects) {
+		} else if (revs.max_count < 0 &&
+			   revs.tag_objects && revs.tree_objects && revs.blob_objects) {
 			if (!prepare_bitmap_walk(&revs)) {
 				traverse_bitmap_commit_list(&show_object_fast);
 				return 0;
 			}
 		}
 	}
