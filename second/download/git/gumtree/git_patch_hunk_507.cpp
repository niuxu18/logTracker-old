 
 static struct ref_dir *get_packed_ref_dir(struct packed_ref_cache *packed_ref_cache)
 {
 	return get_ref_dir(packed_ref_cache->root);
 }
 
-static struct ref_dir *get_packed_refs(struct ref_cache *refs)
+static struct ref_dir *get_packed_refs(struct files_ref_store *refs)
 {
 	return get_packed_ref_dir(get_packed_ref_cache(refs));
 }
 
 /*
  * Add a reference to the in-memory packed reference cache.  This may
  * only be called while the packed-refs file is locked (see
  * lock_packed_refs()).  To actually write the packed-refs file, call
  * commit_packed_refs().
  */
-static void add_packed_ref(const char *refname, const unsigned char *sha1)
+static void add_packed_ref(struct files_ref_store *refs,
+			   const char *refname, const unsigned char *sha1)
 {
-	struct packed_ref_cache *packed_ref_cache =
-		get_packed_ref_cache(&ref_cache);
+	struct packed_ref_cache *packed_ref_cache = get_packed_ref_cache(refs);
 
 	if (!packed_ref_cache->lock)
 		die("internal error: packed refs not locked");
 	add_ref(get_packed_ref_dir(packed_ref_cache),
 		create_ref_entry(refname, sha1, REF_ISPACKED, 1));
 }
