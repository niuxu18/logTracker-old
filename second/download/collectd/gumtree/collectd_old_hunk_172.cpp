
#include "oconfig.h"

extern FILE *yyin;

oconfig_item_t *ci_root;

static void yyset_in  (FILE *fd)
{
  yyin = fd;
} /* void yyset_in */

oconfig_item_t *oconfig_parse_fh (FILE *fh)
{
  int status;
  oconfig_item_t *ret;

  yyset_in (fh);

  status = yyparse ();
  if (status != 0)
  {
    fprintf (stderr, "yyparse returned error #%i\n", status);
    return (NULL);
  }

  ret = ci_root;
  ci_root = NULL;
  yyset_in ((FILE *) 0);

  return (ret);
} /* oconfig_item_t *oconfig_parse_fh */

oconfig_item_t *oconfig_parse_file (const char *file)
{
  FILE *fh;
  oconfig_item_t *ret;

  fh = fopen (file, "r");
  if (fh == NULL)
  {
    fprintf (stderr, "fopen (%s) failed: %s\n", file, strerror (errno));
    return (NULL);
  }

  ret = oconfig_parse_fh (fh);
  fclose (fh);

  return (ret);
} /* oconfig_item_t *oconfig_parse_file */

void oconfig_free (oconfig_item_t *ci)
{
  int i;

  if (ci->values != NULL)
    free (ci->values);

  for (i = 0; i < ci->children_num; i++)
    oconfig_free (ci->children + i);
}

/*
 * vim:shiftwidth=2:tabstop=8:softtabstop=2
 */
