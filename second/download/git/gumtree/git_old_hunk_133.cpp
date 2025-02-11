	N_("git update-ref [options] -d <refname> [<oldval>]"),
	N_("git update-ref [options]    <refname> <newval> [<oldval>]"),
	N_("git update-ref [options] --stdin [-z]"),
	NULL
};

static int updates_alloc;
static int updates_count;
static const struct ref_update **updates;

static char line_termination = '\n';
static int update_flags;

static struct ref_update *update_alloc(void)
{
	struct ref_update *update;

	/* Allocate and zero-init a struct ref_update */
	update = xcalloc(1, sizeof(*update));
	ALLOC_GROW(updates, updates_count + 1, updates_alloc);
	updates[updates_count++] = update;

	/* Store and reset accumulated options */
	update->flags = update_flags;
	update_flags = 0;

	return update;
}

static void update_store_ref_name(struct ref_update *update,
				  const char *ref_name)
{
	if (check_refname_format(ref_name, REFNAME_ALLOW_ONELEVEL))
		die("invalid ref format: %s", ref_name);
	update->ref_name = xstrdup(ref_name);
}

static void update_store_new_sha1(struct ref_update *update,
				  const char *newvalue)
{
	if (*newvalue && get_sha1(newvalue, update->new_sha1))
		die("invalid new value for ref %s: %s",
		    update->ref_name, newvalue);
}

static void update_store_old_sha1(struct ref_update *update,
				  const char *oldvalue)
{
	if (*oldvalue && get_sha1(oldvalue, update->old_sha1))
		die("invalid old value for ref %s: %s",
		    update->ref_name, oldvalue);

	/* We have an old value if non-empty, or if empty without -z */
	update->have_old = *oldvalue || line_termination;
}

static const char *parse_arg(const char *next, struct strbuf *arg)
{
	/* Parse SP-terminated, possibly C-quoted argument */
	if (*next != '"')
		while (*next && !isspace(*next))
			strbuf_addch(arg, *next++);
	else if (unquote_c_style(arg, next, &next))
		die("badly quoted argument: %s", next);

	/* Return position after the argument */
	return next;
}

static const char *parse_first_arg(const char *next, struct strbuf *arg)
{
	/* Parse argument immediately after "command SP" */
	strbuf_reset(arg);
	if (line_termination) {
		/* Without -z, use the next argument */
		next = parse_arg(next, arg);
	} else {
		/* With -z, use rest of first NUL-terminated line */
		strbuf_addstr(arg, next);
		next = next + arg->len;
	}
	return next;
}

static const char *parse_next_arg(const char *next, struct strbuf *arg)
{
	/* Parse next SP-terminated or NUL-terminated argument, if any */
	strbuf_reset(arg);
	if (line_termination) {
		/* Without -z, consume SP and use next argument */
		if (!*next)
			return NULL;
		if (*next != ' ')
			die("expected SP but got: %s", next);
		next = parse_arg(next + 1, arg);
	} else {
		/* With -z, read the next NUL-terminated line */
		if (*next)
			die("expected NUL but got: %s", next);
		if (strbuf_getline(arg, stdin, '\0') == EOF)
			return NULL;
		next = arg->buf + arg->len;
	}
	return next;
}

static void parse_cmd_update(const char *next)
{
	struct strbuf ref = STRBUF_INIT;
	struct strbuf newvalue = STRBUF_INIT;
	struct strbuf oldvalue = STRBUF_INIT;
	struct ref_update *update;

	update = update_alloc();

	if ((next = parse_first_arg(next, &ref)) != NULL && ref.buf[0])
		update_store_ref_name(update, ref.buf);
	else
		die("update line missing <ref>");

	if ((next = parse_next_arg(next, &newvalue)) != NULL)
		update_store_new_sha1(update, newvalue.buf);
	else
		die("update %s missing <newvalue>", ref.buf);

	if ((next = parse_next_arg(next, &oldvalue)) != NULL)
		update_store_old_sha1(update, oldvalue.buf);
	else if(!line_termination)
		die("update %s missing [<oldvalue>] NUL", ref.buf);

	if (next && *next)
		die("update %s has extra input: %s", ref.buf, next);
}

static void parse_cmd_create(const char *next)
{
	struct strbuf ref = STRBUF_INIT;
	struct strbuf newvalue = STRBUF_INIT;
	struct ref_update *update;

	update = update_alloc();
	update->have_old = 1;

	if ((next = parse_first_arg(next, &ref)) != NULL && ref.buf[0])
		update_store_ref_name(update, ref.buf);
	else
		die("create line missing <ref>");

	if ((next = parse_next_arg(next, &newvalue)) != NULL)
		update_store_new_sha1(update, newvalue.buf);
	else
		die("create %s missing <newvalue>", ref.buf);
	if (is_null_sha1(update->new_sha1))
		die("create %s given zero new value", ref.buf);

	if (next && *next)
		die("create %s has extra input: %s", ref.buf, next);
}

static void parse_cmd_delete(const char *next)
{
	struct strbuf ref = STRBUF_INIT;
	struct strbuf oldvalue = STRBUF_INIT;
	struct ref_update *update;

	update = update_alloc();

	if ((next = parse_first_arg(next, &ref)) != NULL && ref.buf[0])
		update_store_ref_name(update, ref.buf);
	else
		die("delete line missing <ref>");

	if ((next = parse_next_arg(next, &oldvalue)) != NULL)
		update_store_old_sha1(update, oldvalue.buf);
	else if(!line_termination)
		die("delete %s missing [<oldvalue>] NUL", ref.buf);
	if (update->have_old && is_null_sha1(update->old_sha1))
		die("delete %s given zero old value", ref.buf);

	if (next && *next)
		die("delete %s has extra input: %s", ref.buf, next);
}

static void parse_cmd_verify(const char *next)
{
	struct strbuf ref = STRBUF_INIT;
	struct strbuf value = STRBUF_INIT;
	struct ref_update *update;

	update = update_alloc();

	if ((next = parse_first_arg(next, &ref)) != NULL && ref.buf[0])
		update_store_ref_name(update, ref.buf);
	else
		die("verify line missing <ref>");

	if ((next = parse_next_arg(next, &value)) != NULL) {
		update_store_old_sha1(update, value.buf);
		update_store_new_sha1(update, value.buf);
	} else if(!line_termination)
		die("verify %s missing [<oldvalue>] NUL", ref.buf);

	if (next && *next)
		die("verify %s has extra input: %s", ref.buf, next);
}

static void parse_cmd_option(const char *next)
{
	if (!strcmp(next, "no-deref"))
		update_flags |= REF_NODEREF;
	else
		die("option unknown: %s", next);
}

static void update_refs_stdin(void)
{
	struct strbuf cmd = STRBUF_INIT;

	/* Read each line dispatch its command */
	while (strbuf_getline(&cmd, stdin, line_termination) != EOF)
		if (!cmd.buf[0])
			die("empty command in input");
		else if (isspace(*cmd.buf))
			die("whitespace before command: %s", cmd.buf);
		else if (starts_with(cmd.buf, "update "))
			parse_cmd_update(cmd.buf + 7);
		else if (starts_with(cmd.buf, "create "))
			parse_cmd_create(cmd.buf + 7);
		else if (starts_with(cmd.buf, "delete "))
			parse_cmd_delete(cmd.buf + 7);
		else if (starts_with(cmd.buf, "verify "))
			parse_cmd_verify(cmd.buf + 7);
		else if (starts_with(cmd.buf, "option "))
			parse_cmd_option(cmd.buf + 7);
		else
			die("unknown command: %s", cmd.buf);

	strbuf_release(&cmd);
}

int cmd_update_ref(int argc, const char **argv, const char *prefix)
{
	const char *refname, *oldval, *msg = NULL;
	unsigned char sha1[20], oldsha1[20];
