void object_array_clear(struct object_array *array)
{
	int i;
	for (i = 0; i < array->nr; i++)
		object_array_release_entry(&array->objects[i]);
	free(array->objects);
	array->objects = NULL;
	array->nr = array->alloc = 0;
}