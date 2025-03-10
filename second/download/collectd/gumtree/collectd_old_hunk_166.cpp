 *  `name'      Name/type of the data-set that describe the values in `vl'.
 *  `vl'        Value list of the values that have been read by a `read'
 *              function.
 */
int plugin_dispatch_values (const char *name, value_list_t *vl);

void plugin_log (int level, const char *format, ...);
#define ERROR(...)   plugin_log (LOG_ERR,     __VA_ARGS__)
#define WARNING(...) plugin_log (LOG_WARNING, __VA_ARGS__)
#define NOTICE(...)  plugin_log (LOG_NOTICE,  __VA_ARGS__)
#define INFO(...)    plugin_log (LOG_INFO,    __VA_ARGS__)
#if COLLECT_DEBUG
# define DEBUG(...)  plugin_log (LOG_DEBUG,   __VA_ARGS__)
#else /* COLLECT_DEBUG */
# define DEBUG(...)  /* noop */
#endif /* ! COLLECT_DEBUG */

/* TODO: Move plugin_{complain,relief} into `utils_complain.[ch]'. -octo */
void plugin_complain (int level, complain_t *c, const char *format, ...);
void plugin_relief (int level, complain_t *c, const char *format, ...);

const data_set_t *plugin_get_ds (const char *name);

#endif /* PLUGIN_H */
