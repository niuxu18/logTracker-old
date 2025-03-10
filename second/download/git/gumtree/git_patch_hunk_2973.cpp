 				continue;
 			branch = branch_get(branch_name);
 
 			refname = branch_get_push(branch, NULL);
 			if (!refname)
 				continue;
-		} else if (starts_with(name, "color:")) {
+		} else if (match_atom_name(name, "color", &valp)) {
 			char color[COLOR_MAXLEN] = "";
 
-			if (color_parse(name + 6, color) < 0)
+			if (!valp)
+				die(_("expected format: %%(color:<color>)"));
+			if (color_parse(valp, color) < 0)
 				die(_("unable to parse format"));
 			v->s = xstrdup(color);
 			continue;
 		} else if (!strcmp(name, "flag")) {
 			char buf[256], *cp = buf;
 			if (ref->flag & REF_ISSYMREF)
