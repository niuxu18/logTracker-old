static CURLcode send_wrong_password(CURL *curl, const char *url, int seq,
                                    long auth_scheme)
{
    return send_request(curl, url, seq, auth_scheme, "testuser:wrongpass");
}