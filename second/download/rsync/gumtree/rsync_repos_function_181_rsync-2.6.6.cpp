static void write_ndx_and_attrs(int f_out, int ndx, int iflags,
				uchar fnamecmp_type, char *buf, int len)
{
	write_int(f_out, ndx);
	if (protocol_version < 29)
		return;
	write_shortint(f_out, iflags);
	if (iflags & ITEM_BASIS_TYPE_FOLLOWS)
		write_byte(f_out, fnamecmp_type);
	if (iflags & ITEM_XNAME_FOLLOWS)
		write_vstring(f_out, buf, len);
}