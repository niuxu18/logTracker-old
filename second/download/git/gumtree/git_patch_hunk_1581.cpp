 	return o;
 }
 
 struct object *deref_tag_noverify(struct object *o)
 {
 	while (o && o->type == OBJ_TAG) {
-		o = parse_object(o->oid.hash);
+		o = parse_object(&o->oid);
 		if (o && o->type == OBJ_TAG && ((struct tag *)o)->tagged)
 			o = ((struct tag *)o)->tagged;
 		else
 			o = NULL;
 	}
 	return o;
 }
 
-struct tag *lookup_tag(const unsigned char *sha1)
+struct tag *lookup_tag(const struct object_id *oid)
 {
-	struct object *obj = lookup_object(sha1);
+	struct object *obj = lookup_object(oid->hash);
 	if (!obj)
-		return create_object(sha1, alloc_tag_node());
+		return create_object(oid->hash, alloc_tag_node());
 	return object_as_type(obj, OBJ_TAG, 0);
 }
 
-static unsigned long parse_tag_date(const char *buf, const char *tail)
+static timestamp_t parse_tag_date(const char *buf, const char *tail)
 {
 	const char *dateptr;
 
 	while (buf < tail && *buf++ != '>')
 		/* nada */;
 	if (buf >= tail)
 		return 0;
 	dateptr = buf;
 	while (buf < tail && *buf++ != '\n')
 		/* nada */;
 	if (buf >= tail)
 		return 0;
-	/* dateptr < buf && buf[-1] == '\n', so strtoul will stop at buf-1 */
-	return strtoul(dateptr, NULL, 10);
+	/* dateptr < buf && buf[-1] == '\n', so parsing will stop at buf-1 */
+	return parse_timestamp(dateptr, NULL, 10);
 }
 
 int parse_tag_buffer(struct tag *item, const void *data, unsigned long size)
 {
-	unsigned char sha1[20];
+	struct object_id oid;
 	char type[20];
 	const char *bufptr = data;
 	const char *tail = bufptr + size;
 	const char *nl;
 
 	if (item->object.parsed)
 		return 0;
 	item->object.parsed = 1;
 
-	if (size < 64)
+	if (size < GIT_SHA1_HEXSZ + 24)
 		return -1;
-	if (memcmp("object ", bufptr, 7) || get_sha1_hex(bufptr + 7, sha1) || bufptr[47] != '\n')
+	if (memcmp("object ", bufptr, 7) || parse_oid_hex(bufptr + 7, &oid, &bufptr) || *bufptr++ != '\n')
 		return -1;
-	bufptr += 48; /* "object " + sha1 + "\n" */
 
 	if (!starts_with(bufptr, "type "))
 		return -1;
 	bufptr += 5;
 	nl = memchr(bufptr, '\n', tail - bufptr);
 	if (!nl || sizeof(type) <= (nl - bufptr))
 		return -1;
 	memcpy(type, bufptr, nl - bufptr);
 	type[nl - bufptr] = '\0';
 	bufptr = nl + 1;
 
 	if (!strcmp(type, blob_type)) {
-		item->tagged = &lookup_blob(sha1)->object;
+		item->tagged = &lookup_blob(&oid)->object;
 	} else if (!strcmp(type, tree_type)) {
-		item->tagged = &lookup_tree(sha1)->object;
+		item->tagged = &lookup_tree(&oid)->object;
 	} else if (!strcmp(type, commit_type)) {
-		item->tagged = &lookup_commit(sha1)->object;
+		item->tagged = &lookup_commit(&oid)->object;
 	} else if (!strcmp(type, tag_type)) {
-		item->tagged = &lookup_tag(sha1)->object;
+		item->tagged = &lookup_tag(&oid)->object;
 	} else {
 		error("Unknown type %s", type);
 		item->tagged = NULL;
 	}
 
 	if (bufptr + 4 < tail && starts_with(bufptr, "tag "))
