	strbuf_insert(&sb, strlen(reflog_msg), ": ", 2);

	transaction = ref_transaction_begin(&err);
	if (!transaction ||
	    ref_transaction_update(transaction, "HEAD", sha1,
				   current_head
				   ? current_head->object.sha1 : NULL,
				   0, !!current_head, sb.buf, &err) ||
	    ref_transaction_commit(transaction, &err)) {
		rollback_index_files();
		die("%s", err.buf);
	}
	ref_transaction_free(transaction);

