static int diff_hunks(mmfile_t *file_a, mmfile_t *file_b, long ctxlen,
		      xdl_emit_hunk_consume_func_t hunk_func, void *cb_data)
{
	xpparam_t xpp = {0};
	xdemitconf_t xecfg = {0};
	xdemitcb_t ecb = {NULL};

	xpp.flags = xdl_opts;
	xecfg.ctxlen = ctxlen;
	xecfg.hunk_func = hunk_func;
	ecb.priv = cb_data;
	return xdi_diff(file_a, file_b, &xpp, &xecfg, &ecb);
}