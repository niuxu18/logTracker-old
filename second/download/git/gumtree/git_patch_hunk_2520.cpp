 		OPT_BIT(0 , "tcl",  &quote_style,
 			N_("quote placeholders suitably for Tcl"), QUOTE_TCL),
 
 		OPT_GROUP(""),
 		OPT_INTEGER( 0 , "count", &maxcount, N_("show only <n> matched refs")),
 		OPT_STRING(  0 , "format", &format, N_("format"), N_("format to use for the output")),
-		OPT_CALLBACK(0 , "sort", sort_tail, N_("key"),
-			    N_("field name to sort on"), &opt_parse_sort),
+		OPT_CALLBACK(0 , "sort", sorting_tail, N_("key"),
+			    N_("field name to sort on"), &parse_opt_ref_sorting),
 		OPT_END(),
 	};
 
 	parse_options(argc, argv, prefix, opts, for_each_ref_usage, 0);
 	if (maxcount < 0) {
 		error("invalid --count argument: `%d'", maxcount);
 		usage_with_options(for_each_ref_usage, opts);
 	}
 	if (HAS_MULTI_BITS(quote_style)) {
 		error("more than one quoting style?");
 		usage_with_options(for_each_ref_usage, opts);
 	}
-	if (verify_format(format))
+	if (verify_ref_format(format))
 		usage_with_options(for_each_ref_usage, opts);
 
-	if (!sort)
-		sort = default_sort();
+	if (!sorting)
+		sorting = ref_default_sorting();
 
 	/* for warn_ambiguous_refs */
 	git_config(git_default_config, NULL);
 
-	memset(&cbdata, 0, sizeof(cbdata));
-	cbdata.grab_pattern = argv;
-	for_each_rawref(grab_single_ref, &cbdata);
-	refs = cbdata.grab_array;
-	num_refs = cbdata.grab_cnt;
-
-	sort_refs(sort, refs, num_refs);
+	memset(&array, 0, sizeof(array));
+	memset(&filter, 0, sizeof(filter));
+	filter.name_patterns = argv;
+	filter_refs(&array, &filter, FILTER_REFS_ALL | FILTER_REFS_INCLUDE_BROKEN);
+	ref_array_sort(sorting, &array);
 
-	if (!maxcount || num_refs < maxcount)
-		maxcount = num_refs;
+	if (!maxcount || array.nr < maxcount)
+		maxcount = array.nr;
 	for (i = 0; i < maxcount; i++)
-		show_ref(refs[i], format, quote_style);
+		show_ref_array_item(array.items[i], format, quote_style);
+	ref_array_clear(&array);
 	return 0;
 }
