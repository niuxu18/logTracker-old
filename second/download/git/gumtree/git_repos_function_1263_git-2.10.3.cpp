void fill_filespec(struct diff_filespec *spec, const unsigned char *sha1,
		   int sha1_valid, unsigned short mode)
{
	if (mode) {
		spec->mode = canon_mode(mode);
		hashcpy(spec->oid.hash, sha1);
		spec->oid_valid = sha1_valid;
	}
}