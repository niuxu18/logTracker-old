packet_buf_write(&preamble, "%s %s\n",
				 oid_to_hex(&ref->old_oid), ref->name);