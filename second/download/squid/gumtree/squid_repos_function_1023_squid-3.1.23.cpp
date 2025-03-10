static void
parseBytesLine64(int64_t * bptr, const char *units)
{
    char *token;
    double d;
    int64_t m;
    int64_t u;

    if ((u = parseBytesUnits(units)) == 0) {
        self_destruct();
        return;
    }

    if ((token = strtok(NULL, w_space)) == NULL) {
        self_destruct();
        return;
    }

    if (strcmp(token, "none") == 0 || strcmp(token, "-1") == 0) {
        *bptr = -1;
        return;
    }

    d = xatof(token);

    m = u;			/* default to 'units' if none specified */

    if (0.0 == d)
        (void) 0;
    else if ((token = strtok(NULL, w_space)) == NULL)
        debugs(3, 0, "WARNING: No units on '" <<
               config_input_line << "', assuming " <<
               d << " " <<  units  );
    else if ((m = parseBytesUnits(token)) == 0) {
        self_destruct();
        return;
    }

    *bptr = static_cast<int64_t>(m * d / u);

    if (static_cast<double>(*bptr) * 2 != m * d / u * 2)
        self_destruct();
}