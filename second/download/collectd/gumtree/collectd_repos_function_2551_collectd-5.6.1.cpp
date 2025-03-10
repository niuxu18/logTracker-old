int meta_data_toc (meta_data_t *md, char ***toc) /* {{{ */
{
  int i = 0, count = 0;

  if ((md == NULL) || (toc == NULL))
    return -EINVAL;

  pthread_mutex_lock (&md->lock);

  for (meta_entry_t *e = md->head; e != NULL; e = e->next)
    ++count;

  if (count == 0)
  {
    pthread_mutex_unlock (&md->lock);
    return (count);
  }

  *toc = calloc(count, sizeof(**toc));
  for (meta_entry_t *e = md->head; e != NULL; e = e->next)
    (*toc)[i++] = strdup(e->key);

  pthread_mutex_unlock (&md->lock);
  return count;
}