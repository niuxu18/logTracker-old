static void parse_sslproxy_ssl_bump(acl_access **ssl_bump)
{
    typedef const char *BumpCfgStyle;
    BumpCfgStyle bcsNone = NULL;
    BumpCfgStyle bcsNew = "new client/server-first/none";
    BumpCfgStyle bcsOld = "deprecated allow/deny";
    static BumpCfgStyle bumpCfgStyleLast = bcsNone;
    BumpCfgStyle bumpCfgStyleNow = bcsNone;
    char *bm;
    if ((bm = ConfigParser::NextToken()) == NULL) {
        self_destruct();
        return;
    }

    // if this is the first rule proccessed
    if (*ssl_bump == NULL) {
        bumpCfgStyleLast = bcsNone;
        sslBumpCfgRr::lastDeprecatedRule = Ssl::bumpEnd;
    }

    allow_t action = allow_t(ACCESS_ALLOWED);

    if (strcmp(bm, Ssl::BumpModeStr[Ssl::bumpClientFirst]) == 0) {
        action.kind = Ssl::bumpClientFirst;
        bumpCfgStyleNow = bcsNew;
    } else if (strcmp(bm, Ssl::BumpModeStr[Ssl::bumpServerFirst]) == 0) {
        action.kind = Ssl::bumpServerFirst;
        bumpCfgStyleNow = bcsNew;
    } else if (strcmp(bm, Ssl::BumpModeStr[Ssl::bumpPeek]) == 0) {
        action.kind = Ssl::bumpPeek;
        bumpCfgStyleNow = bcsNew;
    } else if (strcmp(bm, Ssl::BumpModeStr[Ssl::bumpStare]) == 0) {
        action.kind = Ssl::bumpStare;
        bumpCfgStyleNow = bcsNew;
    } else if (strcmp(bm, Ssl::BumpModeStr[Ssl::bumpSplice]) == 0) {
        action.kind = Ssl::bumpSplice;
        bumpCfgStyleNow = bcsNew;
    } else if (strcmp(bm, Ssl::BumpModeStr[Ssl::bumpBump]) == 0) {
        action.kind = Ssl::bumpBump;
        bumpCfgStyleNow = bcsNew;
    } else if (strcmp(bm, Ssl::BumpModeStr[Ssl::bumpTerminate]) == 0) {
        action.kind = Ssl::bumpTerminate;
        bumpCfgStyleNow = bcsNew;
    } else if (strcmp(bm, Ssl::BumpModeStr[Ssl::bumpNone]) == 0) {
        action.kind = Ssl::bumpNone;
        bumpCfgStyleNow = bcsNew;
    } else if (strcmp(bm, "allow") == 0) {
        debugs(3, DBG_CRITICAL, "SECURITY NOTICE: auto-converting deprecated "
               "\"ssl_bump allow <acl>\" to \"ssl_bump client-first <acl>\" which "
               "is usually inferior to the newer server-first "
               "bumping mode. Update your ssl_bump rules.");
        action.kind = Ssl::bumpClientFirst;
        bumpCfgStyleNow = bcsOld;
        sslBumpCfgRr::lastDeprecatedRule = Ssl::bumpClientFirst;
    } else if (strcmp(bm, "deny") == 0) {
        debugs(3, DBG_CRITICAL, "WARNING: auto-converting deprecated "
               "\"ssl_bump deny <acl>\" to \"ssl_bump none <acl>\". Update "
               "your ssl_bump rules.");
        action.kind = Ssl::bumpNone;
        bumpCfgStyleNow = bcsOld;
        sslBumpCfgRr::lastDeprecatedRule = Ssl::bumpNone;
    } else {
        debugs(3, DBG_CRITICAL, "FATAL: unknown ssl_bump mode: " << bm);
        self_destruct();
        return;
    }

    if (bumpCfgStyleLast != bcsNone && bumpCfgStyleNow != bumpCfgStyleLast) {
        debugs(3, DBG_CRITICAL, "FATAL: do not mix " << bumpCfgStyleNow << " actions with " <<
               bumpCfgStyleLast << " actions. Update your ssl_bump rules.");
        self_destruct();
        return;
    }

    bumpCfgStyleLast = bumpCfgStyleNow;

    // empty rule OK
    ParseAclWithAction(ssl_bump, action, "ssl_bump");
}