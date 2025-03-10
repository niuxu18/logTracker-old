	}
	free(obj_hash);
	obj_hash = new_hash;
	obj_hash_size = new_hash_size;
}

void *create_object(const unsigned char *sha1, int type, void *o)
{
	struct object *obj = o;

	obj->parsed = 0;
	obj->used = 0;
	obj->type = type;
	obj->flags = 0;
	hashcpy(obj->sha1, sha1);

	if (obj_hash_size - 1 <= nr_objs * 2)
		grow_object_hash();

	insert_obj_hash(obj, obj_hash, obj_hash_size);
	nr_objs++;
	return obj;
}

struct object *lookup_unknown_object(const unsigned char *sha1)
{
	struct object *obj = lookup_object(sha1);
	if (!obj)
		obj = create_object(sha1, OBJ_NONE, alloc_object_node());
	return obj;
}

struct object *parse_object_buffer(const unsigned char *sha1, enum object_type type, unsigned long size, void *buffer, int *eaten_p)
{
	struct object *obj;
