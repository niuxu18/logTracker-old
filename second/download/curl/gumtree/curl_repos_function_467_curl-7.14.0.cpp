void curl_slist_free_all(struct curl_slist *list)
{
  struct curl_slist     *next;
  struct curl_slist     *item;

  if (!list)
    return;

  item = list;
  do {
    next = item->next;

    if (item->data) {
      free(item->data);
    }
    free(item);
    item = next;
  } while (next);
}