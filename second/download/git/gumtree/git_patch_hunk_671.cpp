 	 * except if it was in a pack-file and we didn't
 	 * do a full fsck
 	 */
 	if (!(obj->flags & HAS_OBJ)) {
 		if (has_sha1_pack(obj->oid.hash))
 			return; /* it is in pack - forget about it */
-		if (connectivity_only && has_object_file(&obj->oid))
-			return;
-		printf("missing %s %s\n", typename(obj->type),
+		printf("missing %s %s\n", printable_type(obj),
 			describe_object(obj));
 		errors_found |= ERROR_REACHABLE;
 		return;
 	}
 }
 
