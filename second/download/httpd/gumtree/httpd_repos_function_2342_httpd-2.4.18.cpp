static void h2_io_set_sort(h2_io_set *sp)
{
    qsort(sp->list->elts, sp->list->nelts, sp->list->elt_size, 
          h2_stream_id_cmp);
}