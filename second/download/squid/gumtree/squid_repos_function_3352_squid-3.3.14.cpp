void
fatal(const char *message)
{
    /* suppress secondary errors from the dying */
    shutting_down = 1;

    releaseServerSockets();
    /* check for store_dirs_rebuilding because fatal() is often
     * used in early initialization phases, long before we ever
     * get to the store log. */

    /* XXX: this should be turned into a callback-on-fatal, or
     * a mandatory-shutdown-event or something like that.
     * - RBC 20060819
     */

    /*
     * DPW 2007-07-06
     * Call leave_suid() here to make sure that swap.state files
     * are written as the effective user, rather than root.  Squid
     * may take on root privs during reconfigure.  If squid.conf
     * contains a "Bungled" line, fatal() will be called when the
     * process still has root privs.
     */
    leave_suid();

    if (0 == StoreController::store_dirs_rebuilding)
        storeDirWriteCleanLogs(0);

    fatal_common(message);

    exit(1);
}