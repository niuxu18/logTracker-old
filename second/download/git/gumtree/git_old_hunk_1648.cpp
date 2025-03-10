		}
		strbuf_addch(s, *cp);
		cp++;
	}
}

void format_ref_array_item(struct ref_array_item *info, const char *format,
			   int quote_style, struct strbuf *final_buf)
{
	const char *cp, *sp, *ep;
	struct ref_formatting_state state = REF_FORMATTING_STATE_INIT;

	state.quote_style = quote_style;
	push_stack_element(&state.stack);

	for (cp = format; *cp && (sp = find_next(cp)); cp = ep + 1) {
		struct atom_value *atomv;

		ep = strchr(sp, ')');
		if (cp < sp)
			append_literal(cp, sp, &state);
		get_ref_atom_value(info, parse_ref_filter_atom(sp + 2, ep), &atomv);
		atomv->handler(atomv, &state);
	}
	if (*cp) {
		sp = cp + strlen(cp);
		append_literal(cp, sp, &state);
	}
	if (need_color_reset_at_eol) {
		struct atom_value resetv;
		char color[COLOR_MAXLEN] = "";

		if (color_parse("reset", color) < 0)
			die("BUG: couldn't parse 'reset' as a color");
		resetv.s = color;
		append_atom(&resetv, &state);
	}
	if (state.stack->prev)
		die(_("format: %%(end) atom missing"));
	strbuf_addbuf(final_buf, &state.stack->output);
	pop_stack_element(&state.stack);
}

void show_ref_array_item(struct ref_array_item *info, const char *format, int quote_style)
{
	struct strbuf final_buf = STRBUF_INIT;

	format_ref_array_item(info, format, quote_style, &final_buf);
	fwrite(final_buf.buf, 1, final_buf.len, stdout);
	strbuf_release(&final_buf);
	putchar('\n');
}

void pretty_print_ref(const char *name, const unsigned char *sha1,
		const char *format)
{
	struct ref_array_item *ref_item;
	ref_item = new_ref_array_item(name, sha1, 0);
	ref_item->kind = ref_kind_from_refname(name);
	show_ref_array_item(ref_item, format, 0);
	free_array_item(ref_item);
}

/*  If no sorting option is given, use refname to sort as default */
struct ref_sorting *ref_default_sorting(void)
{
	static const char cstr_name[] = "refname";

	struct ref_sorting *sorting = xcalloc(1, sizeof(*sorting));

	sorting->next = NULL;
	sorting->atom = parse_ref_filter_atom(cstr_name, cstr_name + strlen(cstr_name));
	return sorting;
}

int parse_opt_ref_sorting(const struct option *opt, const char *arg, int unset)
{
	struct ref_sorting **sorting_tail = opt->value;
	struct ref_sorting *s;
	int len;

	if (!arg) /* should --no-sort void the list ? */
		return -1;

	s = xcalloc(1, sizeof(*s));
	s->next = *sorting_tail;
	*sorting_tail = s;

	if (*arg == '-') {
		s->reverse = 1;
		arg++;
	}
	if (skip_prefix(arg, "version:", &arg) ||
	    skip_prefix(arg, "v:", &arg))
		s->version = 1;
	len = strlen(arg);
	s->atom = parse_ref_filter_atom(arg, arg+len);
	return 0;
}

int parse_opt_merge_filter(const struct option *opt, const char *arg, int unset)
{
	struct ref_filter *rf = opt->value;
