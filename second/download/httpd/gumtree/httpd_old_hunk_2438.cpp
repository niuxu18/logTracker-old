    {
        sec->port = urld->lud_port? urld->lud_port : LDAP_PORT;
    }

    sec->have_ldap_url = 1;

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, cmd->server,
                 "auth_ldap url parse: `%s', Host: %s, Port: %d, DN: %s, "
                 "attrib: %s, scope: %s, filter: %s, connection mode: %s",
                 url,
                 urld->lud_host,
                 urld->lud_port,
                 urld->lud_dn,
