static int value_list2hv (pTHX_ value_list_t *vl, data_set_t *ds, HV *hash)
{
	AV *values = NULL;
	size_t i;

	if ((NULL == vl) || (NULL == ds) || (NULL == hash))
		return -1;

	values = newAV ();
	/* av_extend takes the last *index* to which the array should be extended. */
	av_extend (values, vl->values_len - 1);

	assert (ds->ds_num == vl->values_len);
	for (i = 0; i < vl->values_len; ++i) {
		SV *val = NULL;

		if (DS_TYPE_COUNTER == ds->ds[i].type)
			val = newSViv (vl->values[i].counter);
		else if (DS_TYPE_GAUGE == ds->ds[i].type)
			val = newSVnv (vl->values[i].gauge);
		else if (DS_TYPE_DERIVE == ds->ds[i].type)
			val = newSViv (vl->values[i].derive);
		else if (DS_TYPE_ABSOLUTE == ds->ds[i].type)
			val = newSViv (vl->values[i].absolute);

		if (NULL == av_store (values, i, val)) {
			av_undef (values);
			return -1;
		}
	}

	if (NULL == hv_store (hash, "values", 6, newRV_noinc ((SV *)values), 0))
		return -1;

	if (0 != vl->time)
	{
		double t = CDTIME_T_TO_DOUBLE (vl->time);
		if (NULL == hv_store (hash, "time", 4, newSVnv (t), 0))
			return -1;
	}

	{
		double t = CDTIME_T_TO_DOUBLE (vl->interval);
		if (NULL == hv_store (hash, "interval", 8, newSVnv (t), 0))
			return -1;
	}

	if ('\0' != vl->host[0])
		if (NULL == hv_store (hash, "host", 4, newSVpv (vl->host, 0), 0))
			return -1;

	if ('\0' != vl->plugin[0])
		if (NULL == hv_store (hash, "plugin", 6, newSVpv (vl->plugin, 0), 0))
			return -1;

	if ('\0' != vl->plugin_instance[0])
		if (NULL == hv_store (hash, "plugin_instance", 15,
				newSVpv (vl->plugin_instance, 0), 0))
			return -1;

	if ('\0' != vl->type[0])
		if (NULL == hv_store (hash, "type", 4, newSVpv (vl->type, 0), 0))
			return -1;

	if ('\0' != vl->type_instance[0])
		if (NULL == hv_store (hash, "type_instance", 13,
				newSVpv (vl->type_instance, 0), 0))
			return -1;
	return 0;
}