 		const char *begin = placeholder + 2;
 		const char *end = strchr(begin, ')');
 		char color[COLOR_MAXLEN];
 
 		if (!end)
 			return 0;
+
 		if (skip_prefix(begin, "auto,", &begin)) {
 			if (!want_color(c->pretty_ctx->color))
 				return end - placeholder + 1;
+		} else if (skip_prefix(begin, "always,", &begin)) {
+			/* nothing to do; we do not respect want_color at all */
+		} else {
+			/* the default is the same as "auto" */
+			if (!want_color(c->pretty_ctx->color))
+				return end - placeholder + 1;
 		}
+
 		if (color_parse_mem(begin, end - begin, color) < 0)
 			die(_("unable to parse --pretty format"));
 		strbuf_addstr(sb, color);
 		return end - placeholder + 1;
 	}
+
+	/*
+	 * We handle things like "%C(red)" above; for historical reasons, there
+	 * are a few colors that can be specified without parentheses (and
+	 * they cannot support things like "auto" or "always" at all).
+	 */
 	if (skip_prefix(placeholder + 1, "red", &rest))
-		strbuf_addstr(sb, GIT_COLOR_RED);
+		basic_color = GIT_COLOR_RED;
 	else if (skip_prefix(placeholder + 1, "green", &rest))
-		strbuf_addstr(sb, GIT_COLOR_GREEN);
+		basic_color = GIT_COLOR_GREEN;
 	else if (skip_prefix(placeholder + 1, "blue", &rest))
-		strbuf_addstr(sb, GIT_COLOR_BLUE);
+		basic_color = GIT_COLOR_BLUE;
 	else if (skip_prefix(placeholder + 1, "reset", &rest))
-		strbuf_addstr(sb, GIT_COLOR_RESET);
+		basic_color = GIT_COLOR_RESET;
+
+	if (basic_color && want_color(c->pretty_ctx->color))
+		strbuf_addstr(sb, basic_color);
+
 	return rest - placeholder;
 }
 
 static size_t parse_padding_placeholder(struct strbuf *sb,
 					const char *placeholder,
 					struct format_commit_context *c)
