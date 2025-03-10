
}

static ssize_t
myread(int fd, void *buf, size_t len)
{
#if _SQUID_WINDOWS_
    return recv(fd, buf, len, 0);
#else
    alarm(io_timeout);
    return read(fd, buf, len);
#endif
}

static ssize_t
mywrite(int fd, void *buf, size_t len)
{
#if _SQUID_WINDOWS_
    return send(fd, buf, len, 0);
#else
    alarm(io_timeout);
    return write(fd, buf, len);
#endif
}

#if HAVE_GSSAPI
/*
 * Check return valuse major_status, minor_status for error and print error description
 * in case of an error.
 * Returns 1 in case of gssapi error
 *         0 in case of no gssapi error
 */
#define BUFFER_SIZE 8192
static int
check_gss_err(OM_uint32 major_status, OM_uint32 minor_status, const char *function)
{
    if (GSS_ERROR(major_status)) {
        OM_uint32 maj_stat, min_stat;
        OM_uint32 msg_ctx = 0;
        gss_buffer_desc status_string;
        char buf[BUFFER_SIZE];
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
                snprintf(buf + len, BUFFER_SIZE-len, "%s", (char *) status_string.value);
                len += status_string.length;
                gss_release_buffer(&min_stat, &status_string);
                break;
            }
            gss_release_buffer(&min_stat, &status_string);
        }
        snprintf(buf + len, BUFFER_SIZE-len, "%s", ". ");
        len += 2;
        msg_ctx = 0;
        while (!msg_ctx) {
            /* convert minor status code (underlying routine error) to text */
            maj_stat = gss_display_status(&min_stat, minor_status,
                                          GSS_C_MECH_CODE,
                                          GSS_C_NULL_OID,
                                          &msg_ctx, &status_string);
            if (maj_stat == GSS_S_COMPLETE) {
                snprintf(buf + len, BUFFER_SIZE-len,"%s", (char *) status_string.value);
                len += status_string.length;
                gss_release_buffer(&min_stat, &status_string);
                break;
            }
            gss_release_buffer(&min_stat, &status_string);
        }
        fprintf(stderr, "%s failed: %s\n", function, buf);
        return (1);
    }
    return (0);
}

/*
 * Get gssapi token for service HTTP/<server>
 * User has to initiate a kinit user@DOMAIN on commandline first for the
 * function to be successful
 * Returns base64 encoded token if successful
 *         string "ERROR" if unsuccessful
 */
static char *
GSSAPI_token(const char *server)
{
    OM_uint32 major_status, minor_status;
    gss_ctx_id_t gss_context = GSS_C_NO_CONTEXT;
    gss_name_t server_name = GSS_C_NO_NAME;
    gss_buffer_desc service = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    char *token = NULL;

    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    if (!server) {
        fprintf(stderr, "Error: No server name\n");
        return (char *)"ERROR";
    }
    service.value = xmalloc(strlen("HTTP") + strlen(server) + 2);
    snprintf((char *) service.value, strlen("HTTP") + strlen(server) + 2, "%s@%s", "HTTP", server);
    service.length = strlen((char *) service.value);

    major_status = gss_import_name(&minor_status, &service,
                                   gss_nt_service_name, &server_name);

    if (!check_gss_err(major_status, minor_status, "gss_import_name()")) {

        major_status = gss_init_sec_context(&minor_status,
                                            GSS_C_NO_CREDENTIAL,
                                            &gss_context,
                                            server_name,
                                            gss_mech_spnego,
                                            0,
                                            0,
                                            GSS_C_NO_CHANNEL_BINDINGS,
                                            &input_token,
                                            NULL,
                                            &output_token,
                                            NULL,
                                            NULL);

        if (!check_gss_err(major_status, minor_status, "gss_init_sec_context()")) {

            if (output_token.length)
                token = (char *) base64_encode_bin((const char *) output_token.value, output_token.length);
        }
    }

    if (!output_token.length)
        token = (char *) "ERROR";
    gss_delete_sec_context(&minor_status, &gss_context, NULL);
    gss_release_buffer(&minor_status, &service);
    gss_release_buffer(&minor_status, &input_token);
    gss_release_buffer(&minor_status, &output_token);
    gss_release_name(&minor_status, &server_name);

    return token;
}
#endif
