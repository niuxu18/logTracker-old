 	if (strbuf_read(&todo_list->buf, fd, 0) < 0) {
 		close(fd);
 		return error(_("could not read '%s'."), todo_file);
 	}
 	close(fd);
 
+	res = stat(todo_file, &st);
+	if (res)
+		return error(_("could not stat '%s'"), todo_file);
+	fill_stat_data(&todo_list->stat, &st);
+
 	res = parse_insn_buffer(todo_list->buf.buf, todo_list);
 	if (res) {
 		if (is_rebase_i(opts))
 			return error(_("please fix this using "
 				       "'git rebase --edit-todo'."));
 		return error(_("unusable instruction sheet: '%s'"), todo_file);
