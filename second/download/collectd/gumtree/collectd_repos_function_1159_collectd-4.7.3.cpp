int meta_data_add_unsigned_int (meta_data_t *md, /* {{{ */
    const char *key, uint64_t value)
{
  meta_entry_t *e;

  if ((md == NULL) || (key == NULL))
    return (-EINVAL);

  e = md_entry_alloc (key);
  if (e == NULL)
    return (-ENOMEM);

  e->value.mv_unsigned_int = value;
  e->type = MD_TYPE_UNSIGNED_INT;

  return (md_entry_insert (md, e));
}