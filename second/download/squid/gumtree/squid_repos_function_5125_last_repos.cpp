bool
Fs::Ufs::UFSSwapDir::pathIsDirectory(const char *aPath)const
{

    struct stat sb;

    if (::stat(aPath, &sb) < 0) {
        int xerrno = errno;
        debugs(47, DBG_CRITICAL, "ERROR: " << aPath << ": " << xstrerr(xerrno));
        return false;
    }

    if (S_ISDIR(sb.st_mode) == 0) {
        debugs(47, DBG_CRITICAL, "WARNING: " << aPath << " is not a directory");
        return false;
    }

    return true;
}