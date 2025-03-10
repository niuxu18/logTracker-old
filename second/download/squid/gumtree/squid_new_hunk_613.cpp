 * src_ip_hash, dst_ip_hash, source_port_hash, dst_port_hash, ports_defined,
 * ports_source, src_ip_alt_hash, dst_ip_alt_hash, src_port_alt_hash, dst_port_alt_hash
 */
static int
parse_wccp2_service_flags(char *flags)
{
    if (!flags)
        return 0;

    char *flag = flags;
    int retflag = 0;

    while (size_t len = strcspn(flag, ",")) {

        if (strncmp(flag, "src_ip_hash", len) == 0) {
            retflag |= WCCP2_SERVICE_SRC_IP_HASH;
        } else if (strncmp(flag, "dst_ip_hash", len) == 0) {
            retflag |= WCCP2_SERVICE_DST_IP_HASH;
        } else if (strncmp(flag, "source_port_hash", len) == 0) {
            retflag |= WCCP2_SERVICE_SRC_PORT_HASH;
        } else if (strncmp(flag, "dst_port_hash", len) == 0) {
            retflag |= WCCP2_SERVICE_DST_PORT_HASH;
        } else if (strncmp(flag, "ports_source", len) == 0) {
            retflag |= WCCP2_SERVICE_PORTS_SOURCE;
        } else if (strncmp(flag, "src_ip_alt_hash", len) == 0) {
            retflag |= WCCP2_SERVICE_SRC_IP_ALT_HASH;
        } else if (strncmp(flag, "dst_ip_alt_hash", len) == 0) {
            retflag |= WCCP2_SERVICE_DST_IP_ALT_HASH;
        } else if (strncmp(flag, "src_port_alt_hash", len) == 0) {
            retflag |= WCCP2_SERVICE_SRC_PORT_ALT_HASH;
        } else if (strncmp(flag, "dst_port_alt_hash", len) == 0) {
            retflag |= WCCP2_SERVICE_DST_PORT_ALT_HASH;
        } else {
            flag[len] = '\0';
            fatalf("Unknown wccp2 service flag: %s\n", flag);
        }

        if (flag[len] == '\0')
            break;

        flag += len+1;
    }

    return retflag;
}

static void
parse_wccp2_service_ports(char *options, int portlist[])
{
    if (!options) {
        return;
    }

    int i = 0;
    char *tmp = options;

    while (size_t len = strcspn(tmp, ",")) {
        if (i >= WCCP2_NUMPORTS) {
            fatalf("parse_wccp2_service_ports: too many ports (maximum: 8) in list '%s'\n", options);
        }
        int p = xatoi(tmp);

        if (p < 1 || p > 65535) {
            fatalf("parse_wccp2_service_ports: port value '%s' isn't valid (1..65535)\n", tmp);
        }

        portlist[i] = p;
        ++i;
        if (tmp[len] == '\0')
            return;
        tmp += len+1;
    }
}

void
parse_wccp2_service_info(void *v)
{
    char *t, *end;
