void
fde::dumpStats (StoreEntry &dumpEntry, int fdNumber)
{
    if (!flags.open)
        return;

#if _SQUID_WINDOWS_

    storeAppendPrintf(&dumpEntry, "%4d 0x%-8lX %-6.6s %4d %7" PRId64 "%c %7" PRId64 "%c %-21s %s\n",
                      fdNumber,
                      win32.handle,
#else
    storeAppendPrintf(&dumpEntry, "%4d %-6.6s %4d %7" PRId64 "%c %7" PRId64 "%c %-21s %s\n",
                      fdNumber,
#endif
                      fdTypeStr[type],
                      timeoutHandler != NULL ? (int) (timeout - squid_curtime) : 0,
                      bytes_read,
                      readPending(fdNumber) ? '*' : ' ',
                      bytes_written,
                      write_handler ? '*' : ' ',
                      remoteAddr(),
                      desc);
}

void
fde::DumpStats (StoreEntry *dumpEntry)
{
    int i;
    storeAppendPrintf(dumpEntry, "Active file descriptors:\n");
#if _SQUID_WINDOWS_

    storeAppendPrintf(dumpEntry, "%-4s %-10s %-6s %-4s %-7s* %-7s* %-21s %s\n",
                      "File",
                      "Handle",
#else
    storeAppendPrintf(dumpEntry, "%-4s %-6s %-4s %-7s* %-7s* %-21s %s\n",
                      "File",
#endif
                      "Type",
                      "Tout",
                      "Nread",
                      "Nwrite",
                      "Remote Address",
                      "Description");
#if _SQUID_WINDOWS_
    storeAppendPrintf(dumpEntry, "---- ---------- ------ ---- -------- -------- --------------------- ------------------------------\n");
#else
    storeAppendPrintf(dumpEntry, "---- ------ ---- -------- -------- --------------------- ------------------------------\n");
#endif

    for (i = 0; i < Squid_MaxFD; ++i) {
        fd_table[i].dumpStats(*dumpEntry, i);
    }
}

char const *
fde::remoteAddr() const
{
    LOCAL_ARRAY(char, buf, MAX_IPSTRLEN );

    if (type != FD_SOCKET)
        return null_string;

    if ( *ipaddr )
        snprintf( buf, MAX_IPSTRLEN, "%s:%d", ipaddr, (int)remote_port);
    else
        local_addr.toUrl(buf,MAX_IPSTRLEN); // toHostStr does not include port.

    return buf;
}

void
fde::noteUse(PconnPool *pool)
{
    ++ pconn.uses;
    pconn.pool = pool;
}