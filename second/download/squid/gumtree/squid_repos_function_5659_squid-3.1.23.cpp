ssize_t
rfc3596BuildAAAAQuery(const char *hostname, char *buf, size_t sz, unsigned short qid, rfc1035_query * query)
{
    return rfc3596BuildHostQuery(hostname, buf, sz, qid, query, RFC1035_TYPE_AAAA);
}