static void show_merge_in_progress(struct wt_status *s,
				struct wt_status_state *state,
				const char *color)
{
	if (has_unmerged(s)) {
		status_printf_ln(s, color, _("You have unmerged paths."));
		if (s->hints)
			status_printf_ln(s, color,
				_("  (fix conflicts and run \"git commit\")"));
	} else {
		status_printf_ln(s, color,
			_("All conflicts fixed but you are still merging."));
		if (s->hints)
			status_printf_ln(s, color,
				_("  (use \"git commit\" to conclude merge)"));
	}
	wt_status_print_trailer(s);
}