				return ACK_common;
			if (strstr(arg, "ready"))
				return ACK_ready;
			return ACK;
		}
	}
	die("git fetch_pack: expected ACK/NAK, got '%s'", line);
}

static void send_request(struct fetch_pack_args *args,
			 int fd, struct strbuf *buf)
{
	if (args->stateless_rpc) {
