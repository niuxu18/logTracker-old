int for_each_ref(each_ref_fn fn, void *cb_data)
{
	return refs_for_each_ref(get_main_ref_store(), fn, cb_data);
}