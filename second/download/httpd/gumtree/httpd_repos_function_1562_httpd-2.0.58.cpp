static char* derive_codepage_from_lang (apr_pool_t *p, char *language)
{
    int lang_len;
    int check_short = 0;
    char *charset;
    
    if (!language)          /* our default codepage */
        return apr_pstrdup(p, "ISO-8859-1");
    else
        lang_len = strlen(language);
    
    charset = (char*) apr_hash_get(charset_conversions, language, APR_HASH_KEY_STRING);

    if (!charset) {
        language[2] = '\0';
        charset = (char*) apr_hash_get(charset_conversions, language, APR_HASH_KEY_STRING);
    }

    if (charset) {
        charset = apr_pstrdup(p, charset);
    }

    return charset;
}