/* Copyright 1999-2005 The Apache Software Foundation or its licensors, as
 * applicable.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Info Module.  Display configuration information for the server and
 * all included modules.
 *
 * <Location /server-info>
 * SetHandler server-info
 * </Location>
 *
 * GET /server-info - Returns full configuration page for server and all modules
 * GET /server-info?server - Returns server configuration only
 * GET /server-info?module_name - Returns configuration for a single module
 * GET /server-info?list - Returns quick list of included modules
 * GET /server-info?config - Returns full configuration
 * GET /server-info?hooks - Returns a listing of the modules active for each hook
 *
 * Original Author:
 *   Rasmus Lerdorf <rasmus vex.net>, May 1996
 *
 * Modified By:
 *   Lou Langholtz <ldl usi.utah.edu>, July 1997
 *
 * Apache 2.0 Port:
 *   Ryan Morgan <rmorgan covalent.net>, August 2000
 *
 */


#include "apr.h"
#include "apr_strings.h"
#include "apr_lib.h"
#define APR_WANT_STRFUNC
#include "apr_want.h"

#define CORE_PRIVATE

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_main.h"
#include "http_protocol.h"
#include "http_connection.h"
#include "http_request.h"
#include "util_script.h"
#include "ap_mpm.h"

typedef struct
{
    const char *name;           /* matching module name */
    const char *info;           /* additional info */
} info_entry;

typedef struct
{
    apr_array_header_t *more_info;
} info_svr_conf;

module AP_MODULE_DECLARE_DATA info_module;

static void *create_info_config(apr_pool_t * p, server_rec * s)
{
    info_svr_conf *conf =
        (info_svr_conf *) apr_pcalloc(p, sizeof(info_svr_conf));

    conf->more_info = apr_array_make(p, 20, sizeof(info_entry));
    return conf;
}

static void *merge_info_config(apr_pool_t * p, void *basev, void *overridesv)
{
    info_svr_conf *new =
        (info_svr_conf *) apr_pcalloc(p, sizeof(info_svr_conf));
    info_svr_conf *base = (info_svr_conf *) basev;
    info_svr_conf *overrides = (info_svr_conf *) overridesv;

    new->more_info =
        apr_array_append(p, overrides->more_info, base->more_info);
    return new;
}

static void put_int_flush_right(request_rec * r, int i, int field)
{
    if (field > 1 || i > 9)
        put_int_flush_right(r, i / 10, field - 1);
    if (i)
        ap_rputc('0' + i % 10, r);
    else
        ap_rputs("&nbsp;", r);
}

static void mod_info_indent(request_rec * r, int nest,
                            const char *thisfn, int linenum)
{
    int i;
    const char *prevfn =
        ap_get_module_config(r->request_config, &info_module);
    if (thisfn == NULL)
        thisfn = "*UNKNOWN*";
    if (prevfn == NULL || 0 != strcmp(prevfn, thisfn)) {
        thisfn = ap_escape_html(r->pool, thisfn);
        ap_rprintf(r, "<dd><tt><strong>In file: %s</strong></tt></dd>\n",
                   thisfn);
        ap_set_module_config(r->request_config, &info_module,
                             (void *) thisfn);
    }

    ap_rputs("<dd><tt>", r);
    put_int_flush_right(r, linenum > 0 ? linenum : 0, 4);
    ap_rputs(":&nbsp;", r);

    for (i = 1; i <= nest; ++i) {
        ap_rputs("&nbsp;&nbsp;", r);
    }
}

static void mod_info_show_cmd(request_rec * r, const ap_directive_t * dir,
                              int nest)
{
    mod_info_indent(r, nest, dir->filename, dir->line_num);
    ap_rprintf(r, "%s <i>%s</i></tt></dd>\n",
               ap_escape_html(r->pool, dir->directive),
               ap_escape_html(r->pool, dir->args));
}

static void mod_info_show_open(request_rec * r, const ap_directive_t * dir,
                               int nest)
{
    mod_info_indent(r, nest, dir->filename, dir->line_num);
    ap_rprintf(r, "%s %s</tt></dd>\n",
               ap_escape_html(r->pool, dir->directive),
               ap_escape_html(r->pool, dir->args));
}

static void mod_info_show_close(request_rec * r, const ap_directive_t * dir,
                                int nest)
{
    const char *dirname = dir->directive;
    mod_info_indent(r, nest, dir->filename, 0);
    if (*dirname == '<') {
        ap_rprintf(r, "&lt;/%s&gt;</tt></dd>",
                   ap_escape_html(r->pool, dirname + 1));
    }
    else {
        ap_rprintf(r, "/%s</tt></dd>", ap_escape_html(r->pool, dirname));
    }
}

static int mod_info_has_cmd(const command_rec * cmds, ap_directive_t * dir)
{
    const command_rec *cmd;
    if (cmds == NULL)
        return 1;
    for (cmd = cmds; cmd->name; ++cmd) {
        if (strcasecmp(cmd->name, dir->directive) == 0)
            return 1;
    }
    return 0;
}

static void mod_info_show_parents(request_rec * r, ap_directive_t * node,
                                  int from, int to)
{
    if (from < to)
        mod_info_show_parents(r, node->parent, from, to - 1);
    mod_info_show_open(r, node, to);
}

static int mod_info_module_cmds(request_rec * r, const command_rec * cmds,
                                ap_directive_t * node, int from, int level)
{
    int shown = from;
    ap_directive_t *dir;
    if (level == 0)
        ap_set_module_config(r->request_config, &info_module, NULL);
    for (dir = node; dir; dir = dir->next) {
        if (dir->first_child != NULL) {
            if (level < mod_info_module_cmds(r, cmds, dir->first_child,
                                             shown, level + 1)) {
                shown = level;
                mod_info_show_close(r, dir, level);
	    }
	}
        else if (mod_info_has_cmd(cmds, dir)) {
            if (shown < level) {
                mod_info_show_parents(r, dir->parent, shown, level - 1);
                shown = level;
            }
            mod_info_show_cmd(r, dir, level);
	}
    }
    return shown;
}

typedef struct
{                               /*XXX: should get something from apr_hooks.h instead */
    void (*pFunc) (void);       /* just to get the right size */
    const char *szName;
    const char *const *aszPredecessors;
    const char *const *aszSuccessors;
    int nOrder;
} hook_struct_t;

/*
 * hook_get_t is a pointer to a function that takes void as an argument and
 * returns a pointer to an apr_array_header_t.  The nasty WIN32 ifdef
 * is required to account for the fact that the ap_hook* calls all use
 * STDCALL calling convention.
 */
typedef apr_array_header_t *(
#ifdef WIN32
                                __stdcall
#endif
                                * hook_get_t)      (void);

typedef struct
{
    const char *name;
    hook_get_t get;
} hook_lookup_t;

static hook_lookup_t startup_hooks[] = {
    {"Pre-Config", ap_hook_get_pre_config},
    {"Test Configuration", ap_hook_get_test_config},
    {"Post Configuration", ap_hook_get_post_config},
    {"Open Logs", ap_hook_get_open_logs},
    {"Child Init", ap_hook_get_child_init},
    {NULL},
};

static hook_lookup_t request_hooks[] = {
    {"Pre-Connection", ap_hook_get_pre_connection},
    {"Create Connection", ap_hook_get_create_connection},
    {"Process Connection", ap_hook_get_process_connection},
    {"Create Request", ap_hook_get_create_request},
    {"Post-Read Request", ap_hook_get_post_read_request},
    {"Header Parse", ap_hook_get_header_parser},
    {"HTTP Scheme", ap_hook_get_http_scheme},
    {"Default Port", ap_hook_get_default_port},
    {"Quick Handler", ap_hook_get_quick_handler},
    {"Translate Name", ap_hook_get_translate_name},
    {"Map to Storage", ap_hook_get_map_to_storage},
    {"Check Access", ap_hook_get_access_checker},
    {"Verify User ID", ap_hook_get_check_user_id},
    {"Verify User Access", ap_hook_get_auth_checker},
    {"Check Type", ap_hook_get_type_checker},
    {"Fixups", ap_hook_get_fixups},
    {"Insert Filters", ap_hook_get_insert_filter},
    {"Content Handlers", ap_hook_get_handler},
    {"Logging", ap_hook_get_log_transaction},
    {"Insert Errors", ap_hook_get_insert_error_filter},
    {NULL},
};

static int module_find_hook(module * modp, hook_get_t hook_get)
{
    int i;
    apr_array_header_t *hooks = hook_get();
    hook_struct_t *elts;

    if (!hooks) {
        return 0;
    }

    elts = (hook_struct_t *) hooks->elts;

    for (i = 0; i < hooks->nelts; i++) {
        if (strcmp(elts[i].szName, modp->name) == 0) {
            return 1;
        }
    }

    return 0;
}

static void module_participate(request_rec * r,
                               module * modp,
                               hook_lookup_t * lookup, int *comma)
{
    if (module_find_hook(modp, lookup->get)) {
        if (*comma) {
            ap_rputs(", ", r);
        }
        ap_rvputs(r, "<tt>", lookup->name, "</tt>", NULL);
        *comma = 1;
    }
}

static void module_request_hook_participate(request_rec * r, module * modp)
{
    int i, comma = 0;

    ap_rputs("<dt><strong>Request Phase Participation:</strong>\n", r);

    for (i = 0; request_hooks[i].name; i++) {
        module_participate(r, modp, &request_hooks[i], &comma);
    }

    if (!comma) {
        ap_rputs("<tt> <em>none</em></tt>", r);
    }
    ap_rputs("</dt>\n", r);
}

static const char *find_more_info(server_rec * s, const char *module_name)
{
    int i;
    info_svr_conf *conf =
        (info_svr_conf *) ap_get_module_config(s->module_config,
                                               &info_module);
    info_entry *entry = (info_entry *) conf->more_info->elts;

    if (!module_name) {
        return 0;
    }
    for (i = 0; i < conf->more_info->nelts; i++) {
