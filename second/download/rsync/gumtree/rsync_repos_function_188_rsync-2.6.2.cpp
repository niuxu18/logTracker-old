void log_recv(struct file_struct *file, struct stats *initial_stats)
{
	if (lp_transfer_logging(module_id)) {
		log_formatted(FLOG, lp_log_format(module_id), "recv", file, initial_stats);
	} else if (log_format && !am_server) {
		log_formatted(FINFO, log_format, "recv", file, initial_stats);
	}
}