				&& unuse_one_window(p))
				; /* nothing */
			win->base = xmmap(NULL, win->len,
				PROT_READ, MAP_PRIVATE,
				p->pack_fd, win->offset);
			if (win->base == MAP_FAILED)
				die_errno("packfile %s cannot be mapped",
					  p->pack_name);
			if (!win->offset && win->len == p->pack_size
				&& !p->do_not_close)
				close_pack_fd(p);
			pack_mmap_calls++;
			pack_open_windows++;
			if (pack_mapped > peak_pack_mapped)
