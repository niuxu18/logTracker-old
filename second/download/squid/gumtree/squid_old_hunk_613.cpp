 * src_ip_hash, dst_ip_hash, source_port_hash, dst_port_hash, ports_defined,
 * ports_source, src_ip_alt_hash, dst_ip_alt_hash, src_port_alt_hash, dst_port_alt_hash
 */
static int
parse_wccp2_service_flags(char *flags)
{
    char *tmp, *tmp2;
    char *flag;
    int retflag = 0;

    if (!flags) {
        return 0;
    }

    tmp = xstrdup(flags);
    tmp2 = tmp;

    flag = strsep(&tmp2, ",");

    while (flag) {
        if (strcmp(flag, "src_ip_hash") == 0) {
            retflag |= WCCP2_SERVICE_SRC_IP_HASH;
        } else if (strcmp(flag, "dst_ip_hash") == 0) {
            retflag |= WCCP2_SERVICE_DST_IP_HASH;
        } else if (strcmp(flag, "source_port_hash") == 0) {
            retflag |= WCCP2_SERVICE_SRC_PORT_HASH;
        } else if (strcmp(flag, "dst_port_hash") == 0) {
            retflag |= WCCP2_SERVICE_DST_PORT_HASH;
        } else if (strcmp(flag, "ports_source") == 0) {
            retflag |= WCCP2_SERVICE_PORTS_SOURCE;
        } else if (strcmp(flag, "src_ip_alt_hash") == 0) {
            retflag |= WCCP2_SERVICE_SRC_IP_ALT_HASH;
        } else if (strcmp(flag, "dst_ip_alt_hash") == 0) {
            retflag |= WCCP2_SERVICE_DST_IP_ALT_HASH;
        } else if (strcmp(flag, "src_port_alt_hash") == 0) {
            retflag |= WCCP2_SERVICE_SRC_PORT_ALT_HASH;
        } else if (strcmp(flag, "dst_port_alt_hash") == 0) {
            retflag |= WCCP2_SERVICE_DST_PORT_ALT_HASH;
        } else {
            fatalf("Unknown wccp2 service flag: %s\n", flag);
        }

        flag = strsep(&tmp2, ",");
    }

    xfree(tmp);
    return retflag;
}

static void
parse_wccp2_service_ports(char *options, int portlist[])
{
    int i = 0;
    int p;
    char *tmp, *tmp2, *port;

    if (!options) {
        return;
    }

    tmp = xstrdup(options);
    tmp2 = tmp;

    port = strsep(&tmp2, ",");

    while (port && i < WCCP2_NUMPORTS) {
        p = xatoi(port);

        if (p < 1 || p > 65535) {
            fatalf("parse_wccp2_service_ports: port value '%s' isn't valid (1..65535)\n", port);
        }

        portlist[i] = p;
        ++i;
        port = strsep(&tmp2, ",");
    }

    if (i == WCCP2_NUMPORTS && port) {
        fatalf("parse_wccp2_service_ports: too many ports (maximum: 8) in list '%s'\n", options);
    }

    xfree(tmp);
}

void
parse_wccp2_service_info(void *v)
{
    char *t, *end;
