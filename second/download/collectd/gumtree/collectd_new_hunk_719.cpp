  {
    ci_copy->children = (oconfig_item_t *)calloc((size_t)ci_orig->children_num,
                                                 sizeof(*ci_copy->children));
    if (ci_copy->children == NULL) {
      fprintf(stderr, "calloc failed.\n");
      oconfig_free(ci_copy);
      return NULL;
    }
    ci_copy->children_num = ci_orig->children_num;

    for (int i = 0; i < ci_copy->children_num; i++) {
      oconfig_item_t *child;

      child = oconfig_clone(ci_orig->children + i);
      if (child == NULL) {
        oconfig_free(ci_copy);
        return NULL;
      }
      child->parent = ci_copy;
      ci_copy->children[i] = *child;
      free(child);
    } /* for (i = 0; i < ci_copy->children_num; i++) */
  }   /* }}} if (ci_orig->children_num > 0) */

  return ci_copy;
} /* oconfig_item_t *oconfig_clone */

static void oconfig_free_all(oconfig_item_t *ci) {
  if (ci == NULL)
    return;

