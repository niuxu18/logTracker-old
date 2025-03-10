void
AIODiskFile::open(int flags, mode_t mode, RefCount<IORequestor> callback)
{
    /* Simulate async calls */
#ifdef _SQUID_WIN32_
    fd = aio_open(path.termedBuf(), flags);
#else
    fd = file_open(path.termedBuf() , flags);
#endif

    ioRequestor = callback;

    if (fd < 0) {
        debugs(79, 3, HERE << ": got failure (" << errno << ")");
        error(true);
    } else {
        closed = false;
        store_open_disk_fd++;
        debugs(79, 3, HERE << ": opened FD " << fd);
    }

    callback->ioCompletedNotification();
}