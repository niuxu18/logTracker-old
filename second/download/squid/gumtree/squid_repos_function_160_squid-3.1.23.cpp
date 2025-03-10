int check_gss_err(OM_uint32 major_status, OM_uint32 minor_status, const char* function, int debug, int log)
{
    if (GSS_ERROR(major_status)) {
        OM_uint32 maj_stat,min_stat;
        OM_uint32 msg_ctx = 0;
        gss_buffer_desc status_string;
        char buf[1024];
        size_t len;

        len = 0;
        msg_ctx = 0;
        while (!msg_ctx) {
            /* convert major status code (GSS-API error) to text */
            maj_stat = gss_display_status(&min_stat, major_status,
                                          GSS_C_GSS_CODE,
                                          GSS_C_NULL_OID,
                                          &msg_ctx, &status_string);
            if (maj_stat == GSS_S_COMPLETE) {
                if (sizeof(buf) > len + status_string.length + 1) {
                    sprintf(buf+len, "%s", (char*) status_string.value);
                    len += status_string.length;
                }
                gss_release_buffer(&min_stat, &status_string);
                break;
            }
            gss_release_buffer(&min_stat, &status_string);
        }
        if (sizeof(buf) > len + 2) {
            sprintf(buf+len, "%s", ". ");
            len += 2;
        }
        msg_ctx = 0;
        while (!msg_ctx) {
            /* convert minor status code (underlying routine error) to text */
            maj_stat = gss_display_status(&min_stat, minor_status,
                                          GSS_C_MECH_CODE,
                                          GSS_C_NULL_OID,
                                          &msg_ctx, &status_string);
            if (maj_stat == GSS_S_COMPLETE) {
                if (sizeof(buf) > len + status_string.length ) {
                    sprintf(buf+len, "%s", (char*) status_string.value);
                    len += status_string.length;
                }
                gss_release_buffer(&min_stat, &status_string);
                break;
            }
            gss_release_buffer(&min_stat, &status_string);
        }
        if (debug)
            fprintf(stderr, "%s| %s: ERROR: %s failed: %s\n", LogTime(), PROGRAM, function, buf);
        fprintf(stdout, "BH %s failed: %s\n",function, buf);
        if (log)
            fprintf(stderr, "%s| %s: INFO: User not authenticated\n", LogTime(), PROGRAM);
        return(1);
    }
    return(0);
}