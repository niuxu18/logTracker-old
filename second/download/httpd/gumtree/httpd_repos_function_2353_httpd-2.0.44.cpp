static const char *match_boyer_moore_horspool_nocase(
                               const apr_strmatch_pattern *this_pattern,
                               const char *s, apr_size_t slen)
{
    const char *s_end = s + slen;
    int *shift = (int *)(this_pattern->context);
    const char *s_next = s + this_pattern->length - 1;
    const char *p_start = this_pattern->pattern;
    const char *p_end = p_start + this_pattern->length - 1;
    while (s_next < s_end) {
        const char *s_tmp = s_next;
        const char *p_tmp = p_end;
        while (apr_tolower(*s_tmp) == apr_tolower(*p_tmp)) {
            p_tmp--;
            if (p_tmp < p_start) {
                return s_tmp;
            }
            s_tmp--;
        }
        s_next += shift[apr_tolower(*s_next)];
    }
    return NULL;
}