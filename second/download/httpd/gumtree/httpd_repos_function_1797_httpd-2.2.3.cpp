static void util_ldap_strdup (char **str, const char *newstr)
{
    if (*str) {
        free(*str);
        *str = NULL;
    }

    if (newstr) {
        *str = strdup(newstr);
    }
}