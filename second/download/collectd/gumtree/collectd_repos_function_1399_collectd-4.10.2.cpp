static int udb_result_handle_result (udb_result_t *r, /* {{{ */
    udb_query_preparation_area_t *q_area,
    udb_result_preparation_area_t *r_area,
    const udb_query_t const *q, char **column_values)
{
  size_t i;

  assert (r && q_area && r_area);

  if (r->legacy_mode == 1)
    return (udb_legacy_result_handle_result (r, q_area, r_area,
          q, column_values));

  assert (r->legacy_mode == 0);

  for (i = 0; i < r->instances_num; i++)
    r_area->instances_buffer[i] = column_values[r_area->instances_pos[i]];

  for (i = 0; i < r->values_num; i++)
    r_area->values_buffer[i] = column_values[r_area->values_pos[i]];

  return udb_result_submit (r, r_area, q, q_area);
}