size_t
headersEnd(const char *mime, size_t l)
{
    size_t e = 0;
    int state = 1;

    PROF_start(headersEnd);

    while (e < l && state < 3) {
        switch (state) {

        case 0:

            if ('\n' == mime[e])
                state = 1;

            break;

        case 1:
            if ('\r' == mime[e])
                state = 2;
            else if ('\n' == mime[e])
                state = 3;
            else
                state = 0;

            break;

        case 2:
            if ('\n' == mime[e])
                state = 3;
            else
                state = 0;

            break;

        default:
            break;
        }

        ++e;
    }
    PROF_stop(headersEnd);

    if (3 == state)
        return e;

    return 0;
}