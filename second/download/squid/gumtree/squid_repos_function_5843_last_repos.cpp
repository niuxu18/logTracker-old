char *
get_nis_password(char *user, char *nisdomain, char *nismap)
{
    static char *val = NULL;
    char *password = NULL;
    int vallen, res;

#ifdef DEBUG
    printf("Domain is set to %s\n", nisdomain);
    printf("YP Map is set to %s\n", nismap);
#endif

    /* Free last entry */
    if (val) {
        free(val);
        val = NULL;
    }

    /* Get NIS entry */
    res = yp_match(nisdomain, nismap, user, strlen(user), &val, &vallen);

    switch (res) {
    case NO_YPERR:
        /* username = */
        (void) strtok(val, ":");
        password = strtok(NULL, ",:");
        return password;
    case YPERR_YPBIND:
        syslog(LOG_ERR, "Squid Authentication through ypbind failure: can't communicate with ypbind");
        return NULL;
    case YPERR_KEY:     /* No such key in map */
        return NULL;
    default:
        return NULL;
    }
}