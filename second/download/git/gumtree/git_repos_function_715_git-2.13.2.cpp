void stop_progress_msg(struct progress **p_progress, const char *msg)
{
	struct progress *progress = *p_progress;
	if (!progress)
		return;
	*p_progress = NULL;
	if (progress->last_value != -1) {
		/* Force the last update */
		char *buf;
		struct throughput *tp = progress->throughput;

		if (tp) {
			unsigned int rate = !tp->avg_misecs ? 0 :
					tp->avg_bytes / tp->avg_misecs;
			throughput_string(&tp->display, tp->curr_total, rate);
		}
		progress_update = 1;
		buf = xstrfmt(", %s.\n", msg);
		display(progress, progress->last_value, buf);
		free(buf);
	}
	clear_progress_signal();
	if (progress->throughput)
		strbuf_release(&progress->throughput->display);
	free(progress->throughput);
	free(progress);
}