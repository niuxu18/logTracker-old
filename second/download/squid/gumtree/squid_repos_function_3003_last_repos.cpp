String
ClientHttpRequest::rangeBoundaryStr() const
{
    const char *key;
    String b(APP_FULLNAME);
    b.append(":",1);
    key = storeEntry()->getMD5Text();
    b.append(key, strlen(key));
    return b;
}