	}

	for (i = 0; i < pending_nr; ++i) {
		struct object *object = pending_e[i].item;

		if (object->type == OBJ_NONE)
			parse_object_or_die(object->sha1, NULL);

		while (object->type == OBJ_TAG) {
			struct tag *tag = (struct tag *) object;

			if (object->flags & UNINTERESTING)
				object_list_insert(object, &haves);
			else
				object_list_insert(object, &wants);

			if (!tag->tagged)
				die("bad tag");
			object = parse_object_or_die(tag->tagged->sha1, NULL);
		}

		if (object->flags & UNINTERESTING)
			object_list_insert(object, &haves);
		else
			object_list_insert(object, &wants);
