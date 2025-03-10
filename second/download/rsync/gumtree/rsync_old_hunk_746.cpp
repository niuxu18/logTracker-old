					return -1;
				if (!r1 && len)
					continue;
			}
		}
		if (r1 <= 0) {
			if (ret > 0) return ret;
			return r1;
		}
		len -= r1;
		buf += r1;
		ret += r1;
	}
	return ret;
}



/* this provides functionality somewhat similar to mmap() but using
   read(). It gives sliding window access to a file. mmap() is not
   used because of the possibility of another program (such as a
   mailer) truncating the file thus giving us a SIGBUS */
struct map_struct *map_file(int fd,OFF_T len)
{
	struct map_struct *map;
	map = new(struct map_struct);
	if (!map) out_of_memory("map_file");

	map->fd = fd;
	map->file_size = len;
	map->p = NULL;
	map->p_size = 0;
	map->p_offset = 0;
	map->p_fd_offset = 0;
	map->p_len = 0;
	map->status = 0;

	return map;
}

/* slide the read window in the file */
char *map_ptr(struct map_struct *map,OFF_T offset,int len)
{
	int nread;
	OFF_T window_start, read_start;
	int window_size, read_size, read_offset;
