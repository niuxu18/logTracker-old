static int show_head_ref(const char *refname, const struct object_id *oid,
			 int flag, void *cb_data)
{
	struct strbuf *buf = cb_data;

	if (flag & REF_ISSYMREF) {
		struct object_id unused;
		const char *target = resolve_ref_unsafe(refname,
							RESOLVE_REF_READING,
							unused.hash, NULL);
		const char *target_nons = strip_namespace(target);

		strbuf_addf(buf, "ref: %s\n", target_nons);
	} else {
		strbuf_addf(buf, "%s\n", oid_to_hex(oid));
	}

	return 0;
}