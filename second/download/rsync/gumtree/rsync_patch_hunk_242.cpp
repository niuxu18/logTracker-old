 	    r = inflate(&rx_strm, Z_NO_FLUSH);
 	    n = CHUNK_SIZE - rx_strm.avail_out;
 	    if (r != Z_OK) {
 		fprintf(FERROR, "inflate returned %d (%d bytes)\n", r, n);
 		exit_cleanup(1);
 	    }
-	    if (rx_strm.avail_out != 0)
+	    if (rx_strm.avail_in == 0)
 		recv_state = r_inflated;
 	    if (n != 0) {
 		*data = dbuf;
 		return n;
 	    }
 	    break;
