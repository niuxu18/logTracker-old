				char *slash = strchr(data + i, '/');
				if (colon && slash && colon < data + posn &&
				    slash < data + posn && colon < slash) {
					okay = 1;
				}
			}
			if (okay) {
				struct strbuf target = STRBUF_INIT;
				strbuf_add(&target, base, serverlen);
				strbuf_add(&target, data + i, posn - i);
				if (!strbuf_strip_suffix(&target, "objects")) {
					warning("ignoring alternate that does"
						" not end in 'objects': %s",
						target.buf);
					strbuf_release(&target);
				} else if (is_alternate_allowed(target.buf)) {
					warning("adding alternate object store: %s",
						target.buf);
					newalt = xmalloc(sizeof(*newalt));
					newalt->next = NULL;
					newalt->base = strbuf_detach(&target, NULL);
					newalt->got_indices = 0;
