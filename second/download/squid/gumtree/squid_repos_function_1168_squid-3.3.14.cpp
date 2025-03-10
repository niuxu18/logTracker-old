static void
parse_access_log(CustomLog ** logs)
{
    const char *filename, *logdef_name;

    CustomLog *cl = (CustomLog *)xcalloc(1, sizeof(*cl));

    if ((filename = strtok(NULL, w_space)) == NULL) {
        self_destruct();
        return;
    }

    if (strcmp(filename, "none") == 0) {
        cl->type = Log::Format::CLF_NONE;
        aclParseAclList(LegacyParser, &cl->aclList);
        while (*logs)
            logs = &(*logs)->next;
        *logs = cl;
        return;
    }

    if ((logdef_name = strtok(NULL, w_space)) == NULL)
        logdef_name = "squid";

    debugs(3, 9, "Log definition name '" << logdef_name << "' file '" << filename << "'");

    cl->filename = xstrdup(filename);

    /* look for the definition pointer corresponding to this name */
    Format::Format *lf = Log::TheConfig.logformats;

    while (lf != NULL) {
        debugs(3, 9, "Comparing against '" << lf->name << "'");

        if (strcmp(lf->name, logdef_name) == 0)
            break;

        lf = lf->next;
    }

    if (lf != NULL) {
        cl->type = Log::Format::CLF_CUSTOM;
        cl->logFormat = lf;
    } else if (strcmp(logdef_name, "auto") == 0) {
        debugs(0, DBG_CRITICAL, "WARNING: Log format 'auto' no longer exists. Using 'squid' instead.");
        cl->type = Log::Format::CLF_SQUID;
    } else if (strcmp(logdef_name, "squid") == 0) {
        cl->type = Log::Format::CLF_SQUID;
    } else if (strcmp(logdef_name, "common") == 0) {
        cl->type = Log::Format::CLF_COMMON;
    } else if (strcmp(logdef_name, "combined") == 0) {
        cl->type = Log::Format::CLF_COMBINED;
#if ICAP_CLIENT
    } else if (strcmp(logdef_name, "icap_squid") == 0) {
        cl->type = Log::Format::CLF_ICAP_SQUID;
#endif
    } else if (strcmp(logdef_name, "useragent") == 0) {
        cl->type = Log::Format::CLF_USERAGENT;
    } else if (strcmp(logdef_name, "referrer") == 0) {
        cl->type = Log::Format::CLF_REFERER;
    } else {
        debugs(3, DBG_CRITICAL, "Log format '" << logdef_name << "' is not defined");
        self_destruct();
        return;
    }

    aclParseAclList(LegacyParser, &cl->aclList);

    while (*logs)
        logs = &(*logs)->next;

    *logs = cl;
}