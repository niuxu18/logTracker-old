static void
ftpReadType(Ftp::Gateway * ftpState)
{
    int code = ftpState->ctrl.replycode;
    char *path;
    char *d, *p;
    debugs(9, 3, HERE << "code=" << code);

    if (code == 200) {
        p = path = SBufToCstring(ftpState->request->url.path());

        if (*p == '/')
            ++p;

        while (*p) {
            d = p;
            p += strcspn(p, "/");

            if (*p) {
                *p = '\0';
                ++p;
            }

            rfc1738_unescape(d);

            if (*d)
                wordlistAdd(&ftpState->pathcomps, d);
        }

        xfree(path);

        if (ftpState->pathcomps)
            ftpTraverseDirectory(ftpState);
        else
            ftpListDir(ftpState);
    } else {
        ftpFail(ftpState);
    }
}