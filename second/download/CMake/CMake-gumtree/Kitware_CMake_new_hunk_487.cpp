 * @param yyscanner The scanner object.
 * @return the newly allocated buffer state object.
 */
YY_BUFFER_STATE cmListFileLexer_yy_scan_bytes  (yyconst char * yybytes, yy_size_t  _yybytes_len , yyscan_t yyscanner)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n;
	yy_size_t i;

	/* Get memory for full buffer, including space for trailing EOB's. */
	n = _yybytes_len + 2;
	buf = (char *) cmListFileLexer_yyalloc(n ,yyscanner );
	if ( ! buf )
		YY_FATAL_ERROR( "out of dynamic memory in cmListFileLexer_yy_scan_bytes()" );

	for ( i = 0; i < _yybytes_len; ++i )
		buf[i] = yybytes[i];

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;

	b = cmListFileLexer_yy_scan_buffer(buf,n ,yyscanner);
	if ( ! b )
		YY_FATAL_ERROR( "bad buffer in cmListFileLexer_yy_scan_bytes()" );

	/* It's okay to grow etc. this buffer, and we should throw it
	 * away when we're done.
	 */
	b->yy_is_our_buffer = 1;

	return b;
}

#ifndef YY_EXIT_FAILURE
