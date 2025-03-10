 **/

#include "collectd.h"
#include "common.h"
#include "plugin.h"

#define print_to_socket(fh, ...) \
  if (fprintf (fh, __VA_ARGS__) < 0) { \
    char errbuf[1024]; \
    WARNING ("handle_putnotif: failed to write to socket #%i: %s", \
	fileno (fh), sstrerror (errno, errbuf, sizeof (errbuf))); \
    return -1; \
  }

static int parse_option_severity (notification_t *n, char *value)
{
  if (strcasecmp (value, "Failure") == 0)
    n->severity = NOTIF_FAILURE;
  else if (strcasecmp (value, "Warning") == 0)
    n->severity = NOTIF_WARNING;
  else if (strcasecmp (value, "Okay") == 0)
    n->severity = NOTIF_OKAY;
  else
    return (-1);

  return (0);
} /* int parse_option_severity */

static int parse_option_time (notification_t *n, char *value)
{
  time_t tmp;
  
  tmp = (time_t) atoi (value);
  if (tmp <= 0)
    return (-1);

  n->time = tmp;

  return (0);
} /* int parse_option_time */

static int parse_option (notification_t *n, char *buffer)
{
  char *option = buffer;
  char *value;

  if ((n == NULL) || (option == NULL))
    return (-1);

  value = strchr (option, '=');
  if (value == NULL)
    return (-1);
  *value = '\0'; value++;

  if (strcasecmp ("severity", option) == 0)
    return (parse_option_severity (n, value));
  else if (strcasecmp ("time", option) == 0)
    return (parse_option_time (n, value));
  else if (strcasecmp ("host", option) == 0)
    sstrncpy (n->host, value, sizeof (n->host));
  else if (strcasecmp ("plugin", option) == 0)
    sstrncpy (n->plugin, value, sizeof (n->plugin));
  else if (strcasecmp ("plugin_instance", option) == 0)
    sstrncpy (n->plugin_instance, value, sizeof (n->plugin_instance));
