static void flush_buffer (void)
{
	DEBUG ("network plugin: flush_buffer: send_buffer_fill = %i",
			send_buffer_fill);

	network_send_buffer (send_buffer, (size_t) send_buffer_fill);
	network_init_buffer ();
}