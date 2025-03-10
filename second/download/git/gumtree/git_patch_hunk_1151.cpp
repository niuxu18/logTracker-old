 		if (revs->tag_objects && !(flags & UNINTERESTING))
 			add_pending_object(revs, object, tag->tag);
 		if (!tag->tagged)
 			die("bad tag");
 		object = parse_object(tag->tagged->oid.hash);
 		if (!object) {
-			if (flags & UNINTERESTING)
+			if (revs->ignore_missing_links || (flags & UNINTERESTING))
 				return NULL;
 			die("bad object %s", oid_to_hex(&tag->tagged->oid));
 		}
 		object->flags |= flags;
 		/*
 		 * We'll handle the tagged object by looping or dropping
