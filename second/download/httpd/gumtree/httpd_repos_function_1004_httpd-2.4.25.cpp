static const char* ap_lua_interpolate_string(apr_pool_t* pool, const char* string, const char** values)
{
    char *stringBetween;
    const char* ret;
    int srclen,x,y;
    srclen = strlen(string);
    ret = "";
    y = 0;
    for (x=0; x < srclen; x++) {
        if (string[x] == '$' && x != srclen-1 && string[x+1] >= '0' && string[x+1] <= '9') {
            int v = *(string+x+1) - '0';
            if (x-y > 0) {
                stringBetween = apr_pstrndup(pool, string+y, x-y);
            }
            else {
                stringBetween = "";
            }
            ret = apr_pstrcat(pool, ret, stringBetween, values[v], NULL);
            y = ++x+1;
        }
    }
    
    if (x-y > 0 && y > 0) {
        stringBetween = apr_pstrndup(pool, string+y, x-y);
        ret = apr_pstrcat(pool, ret, stringBetween, NULL);
    }
    /* If no replacement was made, just return the original string */
    else if (y == 0) {
        return string;
    }
    return ret;
}