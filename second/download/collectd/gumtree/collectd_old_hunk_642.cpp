
#include "collectd.h"

#include "common.h"
#include "plugin.h"

#include "utils_cmd_listval.h"
#include "utils_cache.h"
#include "utils_parse_option.h"

#define free_everything_and_return(status) do { \
    for (size_t j = 0; j < number; j++) { \
      sfree(names[j]); \
      names[j] = NULL; \
    } \
    sfree(names); \
    sfree(times); \
    return (status); \
  } while (0)

#define print_to_socket(fh, ...) \
  do { \
    if (fprintf (fh, __VA_ARGS__) < 0) { \
      char errbuf[1024]; \
      WARNING ("handle_listval: failed to write to socket #%i: %s", \
          fileno (fh), sstrerror (errno, errbuf, sizeof (errbuf))); \
      free_everything_and_return (-1); \
    } \
    fflush(fh); \
  } while (0)

int handle_listval (FILE *fh, char *buffer)
{
  char *command;
  char **names = NULL;
  cdtime_t *times = NULL;
  size_t number = 0;
  int status;

  DEBUG ("utils_cmd_listval: handle_listval (fh = %p, buffer = %s);",
      (void *) fh, buffer);

  command = NULL;
  status = parse_string (&buffer, &command);
  if (status != 0)
  {
    print_to_socket (fh, "-1 Cannot parse command.\n");
    free_everything_and_return (-1);
  }
  assert (command != NULL);

  if (strcasecmp ("LISTVAL", command) != 0)
  {
    print_to_socket (fh, "-1 Unexpected command: `%s'.\n", command);
    free_everything_and_return (-1);
  }

  if (*buffer != 0)
  {
    print_to_socket (fh, "-1 Garbage after end of command: %s\n", buffer);
    free_everything_and_return (-1);
  }

  status = uc_get_names (&names, &times, &number);
  if (status != 0)
  {
    DEBUG ("command listval: uc_get_names failed with status %i", status);
    print_to_socket (fh, "-1 uc_get_names failed.\n");
    free_everything_and_return (-1);
  }

  print_to_socket (fh, "%i Value%s found\n",
      (int) number, (number == 1) ? "" : "s");
  for (size_t i = 0; i < number; i++)
    print_to_socket (fh, "%.3f %s\n", CDTIME_T_TO_DOUBLE (times[i]),
       	names[i]);

  free_everything_and_return (0);
} /* int handle_listval */

/* vim: set sw=2 sts=2 ts=8 : */
