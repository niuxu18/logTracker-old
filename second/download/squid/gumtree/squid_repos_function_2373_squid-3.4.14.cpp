static int
idnsFromKnownNameserver(Ip::Address const &from)
{
    int i;

    for (i = 0; i < nns; ++i) {
        if (nameservers[i].S != from)
            continue;

        if (nameservers[i].S.port() != from.port())
            continue;

        return i;
    }

    return -1;
}