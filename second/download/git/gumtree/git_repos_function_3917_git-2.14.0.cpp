static void graph_output_commit_line(struct git_graph *graph, struct strbuf *sb)
{
	int seen_this = 0;
	int i, chars_written;

	/*
	 * Output the row containing this commit
	 * Iterate up to and including graph->num_columns,
	 * since the current commit may not be in any of the existing
	 * columns.  (This happens when the current commit doesn't have any
	 * children that we have already processed.)
	 */
	seen_this = 0;
	chars_written = 0;
	for (i = 0; i <= graph->num_columns; i++) {
		struct column *col = &graph->columns[i];
		struct commit *col_commit;
		if (i == graph->num_columns) {
			if (seen_this)
				break;
			col_commit = graph->commit;
		} else {
			col_commit = graph->columns[i].commit;
		}

		if (col_commit == graph->commit) {
			seen_this = 1;
			graph_output_commit_char(graph, sb);
			chars_written++;

			if (graph->num_parents > 2)
				chars_written += graph_draw_octopus_merge(graph,
									  sb);
		} else if (seen_this && (graph->num_parents > 2)) {
			strbuf_write_column(sb, col, '\\');
			chars_written++;
		} else if (seen_this && (graph->num_parents == 2)) {
			/*
			 * This is a 2-way merge commit.
			 * There is no GRAPH_PRE_COMMIT stage for 2-way
			 * merges, so this is the first line of output
			 * for this commit.  Check to see what the previous
			 * line of output was.
			 *
			 * If it was GRAPH_POST_MERGE, the branch line
			 * coming into this commit may have been '\',
			 * and not '|' or '/'.  If so, output the branch
			 * line as '\' on this line, instead of '|'.  This
			 * makes the output look nicer.
			 */
			if (graph->prev_state == GRAPH_POST_MERGE &&
			    graph->prev_commit_index < i)
				strbuf_write_column(sb, col, '\\');
			else
				strbuf_write_column(sb, col, '|');
			chars_written++;
		} else {
			strbuf_write_column(sb, col, '|');
			chars_written++;
		}
		strbuf_addch(sb, ' ');
		chars_written++;
	}

	graph_pad_horizontally(graph, sb, chars_written);

	/*
	 * Update graph->state
	 */
	if (graph->num_parents > 1)
		graph_update_state(graph, GRAPH_POST_MERGE);
	else if (graph_is_mapping_correct(graph))
		graph_update_state(graph, GRAPH_PADDING);
	else
		graph_update_state(graph, GRAPH_COLLAPSING);
}