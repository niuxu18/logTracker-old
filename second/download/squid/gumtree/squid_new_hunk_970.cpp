    char *path;
    glob_t globbuf;
    int i;
    memset(&globbuf, 0, sizeof(globbuf));
    for (path = strwordtok(files, &saveptr); path; path = strwordtok(NULL, &saveptr)) {
        if (glob(path, globbuf.gl_pathc ? GLOB_APPEND : 0, NULL, &globbuf) != 0) {
            int xerrno = errno;
            fatalf("Unable to find configuration file: %s: %s", path, xstrerr(xerrno));
        }
    }
    for (i = 0; i < (int)globbuf.gl_pathc; ++i) {
        error_count += parseOneConfigFile(globbuf.gl_pathv[i], depth);
    }
    globfree(&globbuf);
