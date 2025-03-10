			tx_strm.zalloc = NULL;
			tx_strm.zfree = NULL;
			if (deflateInit2(&tx_strm, compression_level,
					 Z_DEFLATED, -15, 8,
					 Z_DEFAULT_STRATEGY) != Z_OK) {
				rprintf(FERROR, "compression init failed\n");
				exit_cleanup(RERR_STREAMIO);
			}
			if ((obuf = new_array(char, OBUF_SIZE)) == NULL)
				out_of_memory("send_deflated_token");
			init_done = 1;
		} else
			deflateReset(&tx_strm);
