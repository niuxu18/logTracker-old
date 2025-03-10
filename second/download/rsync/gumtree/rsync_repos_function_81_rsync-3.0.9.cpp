static struct idlist *add_to_list(struct idlist **root, id_t id, const char *name,
				  id_t id2, uint16 flags)
{
	struct idlist *node = new(struct idlist);
	if (!node)
		out_of_memory("add_to_list");
	node->next = *root;
	node->name = name;
	node->id = id;
	node->id2 = id2;
	node->flags = flags;
	*root = node;
	return node;
}