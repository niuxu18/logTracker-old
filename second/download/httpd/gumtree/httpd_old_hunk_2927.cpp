     * Test our assumption that the pid is 32-bits.  It's possible that
     * 64-bit machines will declare pid_t to be 64 bits but only use 32
     * of them.  It would have been really nice to test this during
     * global_init ... but oh well.
     */
    if ((pid_t)cur_unique_id.pid != pid) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s,
                    "oh no! pids are greater than 32-bits!  I'm broken!");
    }

    cur_unique_id.in_addr = global_in_addr;

    /*
     * If we use 0 as the initial counter we have a little less protection
     * against restart problems, and a little less protection against a clock
     * going backwards in time.
     */
    tv = apr_time_now();
    /* Some systems have very low variance on the low end of their system
     * counter, defend against that.
     */
    cur_unique_id.counter = (unsigned short)(apr_time_usec(tv) / 10);

    /*
     * We must always use network ordering for these bytes, so that
     * identifiers are comparable between machines of different byte
     * orderings.  Note in_addr is already in network order.
     */
    cur_unique_id.pid = htonl(cur_unique_id.pid);
    cur_unique_id.counter = htons(cur_unique_id.counter);
}

/* NOTE: This is *NOT* the same encoding used by base64encode ... the last two
 * characters should be + and /.  But those two characters have very special
 * meanings in URLs, and we want to make it easy to use identifiers in
 * URLs.  So we replace them with @ and -.
