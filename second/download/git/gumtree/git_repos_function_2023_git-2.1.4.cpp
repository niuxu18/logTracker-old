static void add_name_decoration(enum decoration_type type, const char *name, struct object *obj)
{
	int nlen = strlen(name);
	struct name_decoration *res = xmalloc(sizeof(struct name_decoration) + nlen);
	memcpy(res->name, name, nlen + 1);
	res->type = type;
	res->next = add_decoration(&name_decoration, obj, res);
}