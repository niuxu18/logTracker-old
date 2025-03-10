int ref_transaction_update(struct ref_transaction *transaction,
			   const char *refname,
			   const unsigned char *new_sha1,
			   const unsigned char *old_sha1,
			   int flags, int have_old,
			   struct strbuf *err)
{
	struct ref_update *update;

	if (have_old && !old_sha1)
		die("BUG: have_old is true but old_sha1 is NULL");

	update = add_update(transaction, refname);
	hashcpy(update->new_sha1, new_sha1);
	update->flags = flags;
	update->have_old = have_old;
	if (have_old)
		hashcpy(update->old_sha1, old_sha1);
	return 0;
}