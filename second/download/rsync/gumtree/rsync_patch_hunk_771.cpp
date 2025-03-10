 			name, host, addr);
 	}
 
 	if (!claim_connection(lp_lock_file(i), lp_max_connections(i))) {
 		if (errno) {
 			rsyserr(FLOG, errno, "failed to open lock file %s",
-				lp_lock_file(i));
-			io_printf(f_out, "@ERROR: failed to open lock file %s\n",
-				  lp_lock_file(i));
+				safe_fname(lp_lock_file(i)));
+			io_printf(f_out, "@ERROR: failed to open lock file\n");
 		} else {
 			rprintf(FLOG, "max connections (%d) reached\n",
 				lp_max_connections(i));
-			io_printf(f_out, "@ERROR: max connections (%d) reached - try again later\n",
+			io_printf(f_out, "@ERROR: max connections (%d) reached -- try again later\n",
 				lp_max_connections(i));
 		}
 		return -1;
 	}
 
 	auth_user = auth_server(f_in, f_out, i, addr, "@RSYNCD: AUTHREQD ");
