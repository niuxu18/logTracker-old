 	if (obj->flags & REACHABLE)
 		return 0;
 	obj->flags |= REACHABLE;
 	if (!(obj->flags & HAS_OBJ)) {
 		if (parent && !has_object_file(&obj->oid)) {
 			printf("broken link from %7s %s\n",
-				 typename(parent->type), oid_to_hex(&parent->oid));
+				 typename(parent->type), describe_object(parent));
 			printf("              to %7s %s\n",
-				 typename(obj->type), oid_to_hex(&obj->oid));
+				 typename(obj->type), describe_object(obj));
 			errors_found |= ERROR_REACHABLE;
 		}
 		return 1;
 	}
 
 	add_object_array(obj, NULL, &pending);
