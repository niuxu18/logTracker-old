(datasize >= (2 + uidsize + 3)) {
					/* get a gid size. */
					gidsize = 0xff & (int)p[offset+2+uidsize];
					if (gidsize == 2)
						zip_entry->gid =
						    archive_le16dec(
						        p+offset+2+uidsize+1);
					else if (gidsize == 4 &&
					    datasize >= (2 + uidsize + 5))
						zip_entry->gid =
						    archive_le32dec(
						        p+offset+2+uidsize+1);
				}