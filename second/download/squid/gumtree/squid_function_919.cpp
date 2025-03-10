int
main(int argc, char *const argv[])
{
    char buf[MAX_AUTHTOKEN_LEN];
    char *c, *p;
    char *user = NULL;
    char *rfc_user = NULL;
#if HAVE_PAC_SUPPORT
    char ad_groups[MAX_PAC_GROUP_SIZE];
    char *ag=NULL;
    krb5_context context = NULL;
    krb5_error_code ret;
    krb5_pac pac;
#if HAVE_HEIMDAL_KERBEROS
    gss_buffer_desc data_set = GSS_C_EMPTY_BUFFER;
#else
    gss_buffer_desc type_id = GSS_C_EMPTY_BUFFER;
#endif
#endif
    long length = 0;
    static int err = 0;
    int opt, log = 0, norealm = 0;
    OM_uint32 ret_flags = 0, spnego_flag = 0;
    char *service_name = (char *) "HTTP", *host_name = NULL;
    char *token = NULL;
    char *service_principal = NULL;
    OM_uint32 major_status, minor_status;
    gss_ctx_id_t gss_context = GSS_C_NO_CONTEXT;
    gss_name_t client_name = GSS_C_NO_NAME;
    gss_name_t server_name = GSS_C_NO_NAME;
    gss_cred_id_t server_creds = GSS_C_NO_CREDENTIAL;
    gss_buffer_desc service = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    const unsigned char *kerberosToken = NULL;
    const unsigned char *spnegoToken = NULL;
    size_t spnegoTokenLength = 0;

    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    while (-1 != (opt = getopt(argc, argv, "dirs:h"))) {
        switch (opt) {
        case 'd':
            debug_enabled = 1;
            break;
        case 'i':
            log = 1;
            break;
        case 'r':
            norealm = 1;
            break;
        case 's':
            service_principal = xstrdup(optarg);
            break;
        case 'h':
            fprintf(stderr, "Usage: \n");
            fprintf(stderr, "squid_kerb_auth [-d] [-i] [-s SPN] [-h]\n");
            fprintf(stderr, "-d full debug\n");
            fprintf(stderr, "-i informational messages\n");
            fprintf(stderr, "-r remove realm from username\n");
            fprintf(stderr, "-s service principal name\n");
            fprintf(stderr, "-h help\n");
            fprintf(stderr,
                    "The SPN can be set to GSS_C_NO_NAME to allow any entry from keytab\n");
            fprintf(stderr, "default SPN is HTTP/fqdn@DEFAULT_REALM\n");
            exit(0);
        default:
            fprintf(stderr, "%s| %s: WARNING: unknown option: -%c.\n", LogTime(),
                    PROGRAM, opt);
        }
    }

    debug((char *) "%s| %s: INFO: Starting version %s\n", LogTime(), PROGRAM, SQUID_KERB_AUTH_VERSION);
    if (service_principal && strcasecmp(service_principal, "GSS_C_NO_NAME")) {
        service.value = service_principal;
        service.length = strlen((char *) service.value);
    } else {
        host_name = gethost_name();
        if (!host_name) {
            fprintf(stderr,
                    "%s| %s: FATAL: Local hostname could not be determined. Please specify the service principal\n",
                    LogTime(), PROGRAM);
            fprintf(stdout, "BH hostname error\n");
            exit(-1);
        }
        service.value = xmalloc(strlen(service_name) + strlen(host_name) + 2);
        snprintf((char *) service.value, strlen(service_name) + strlen(host_name) + 2,
                 "%s@%s", service_name, host_name);
        service.length = strlen((char *) service.value);
        xfree(host_name);
    }

    while (1) {
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
            if (ferror(stdin)) {
                debug((char *) "%s| %s: FATAL: fgets() failed! dying..... errno=%d (%s)\n",
                      LogTime(), PROGRAM, ferror(stdin),
                      strerror(ferror(stdin)));

                fprintf(stdout, "BH input error\n");
                exit(1);	/* BIIG buffer */
            }
            fprintf(stdout, "BH input error\n");
            exit(0);
        }
        c = (char *) memchr(buf, '\n', sizeof(buf) - 1);
        if (c) {
            *c = '\0';
            length = c - buf;
        } else {
            err = 1;
        }
        if (err) {
            debug((char *) "%s| %s: ERROR: Oversized message\n", LogTime(), PROGRAM);
            fprintf(stdout, "BH Oversized message\n");
            err = 0;
            continue;
        }
        debug((char *) "%s| %s: DEBUG: Got '%s' from squid (length: %ld).\n", LogTime(), PROGRAM, buf, length);

        if (buf[0] == '\0') {
            debug((char *) "%s| %s: ERROR: Invalid request\n", LogTime(), PROGRAM);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (strlen(buf) < 2) {
            debug((char *) "%s| %s: ERROR: Invalid request [%s]\n", LogTime(), PROGRAM, buf);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (!strncmp(buf, "QQ", 2)) {
            gss_release_buffer(&minor_status, &input_token);
            gss_release_buffer(&minor_status, &output_token);
            gss_release_buffer(&minor_status, &service);
            gss_release_cred(&minor_status, &server_creds);
            if (server_name)
                gss_release_name(&minor_status, &server_name);
            if (client_name)
                gss_release_name(&minor_status, &client_name);
            if (gss_context != GSS_C_NO_CONTEXT)
                gss_delete_sec_context(&minor_status, &gss_context, NULL);
            if (kerberosToken) {
                /* Allocated by parseNegTokenInit, but no matching free function exists.. */
                if (!spnego_flag)
                    xfree(kerberosToken);
            }
            if (spnego_flag) {
                /* Allocated by makeNegTokenTarg, but no matching free function exists.. */
                xfree(spnegoToken);
            }
            xfree(token);
            fprintf(stdout, "BH quit command\n");
            exit(0);
        }
        if (strncmp(buf, "YR", 2) && strncmp(buf, "KK", 2)) {
            debug((char *) "%s| %s: ERROR: Invalid request [%s]\n", LogTime(), PROGRAM, buf);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (!strncmp(buf, "YR", 2)) {
            if (gss_context != GSS_C_NO_CONTEXT)
                gss_delete_sec_context(&minor_status, &gss_context, NULL);
            gss_context = GSS_C_NO_CONTEXT;
        }
        if (strlen(buf) <= 3) {
            debug((char *) "%s| %s: ERROR: Invalid negotiate request [%s]\n", LogTime(), PROGRAM, buf);
            fprintf(stdout, "BH Invalid negotiate request\n");
            continue;
        }
        input_token.length = (size_t)base64_decode_len(buf+3);
        debug((char *) "%s| %s: DEBUG: Decode '%s' (decoded length: %d).\n",
              LogTime(), PROGRAM, buf + 3, (int) input_token.length);
        input_token.value = xmalloc(input_token.length);

        input_token.length = (size_t)base64_decode((char *) input_token.value, (unsigned int)input_token.length, buf+3);

        if ((input_token.length >= sizeof ntlmProtocol + 1) &&
                (!memcmp(input_token.value, ntlmProtocol, sizeof ntlmProtocol))) {
            debug((char *) "%s| %s: WARNING: received type %d NTLM token\n",
                  LogTime(), PROGRAM,
                  (int) *((unsigned char *) input_token.value +
                          sizeof ntlmProtocol));
            fprintf(stdout, "BH received type %d NTLM token\n",
                    (int) *((unsigned char *) input_token.value +
                            sizeof ntlmProtocol));
            goto cleanup;
        }
        if (service_principal) {
            if (strcasecmp(service_principal, "GSS_C_NO_NAME")) {
                major_status = gss_import_name(&minor_status, &service,
                                               (gss_OID) GSS_C_NULL_OID, &server_name);

            } else {
                server_name = GSS_C_NO_NAME;
                major_status = GSS_S_COMPLETE;
                minor_status = 0;
            }
        } else {
            major_status = gss_import_name(&minor_status, &service,
                                           gss_nt_service_name, &server_name);
        }

        if (check_gss_err(major_status, minor_status, "gss_import_name()", log, 1))
            goto cleanup;

        major_status =
            gss_acquire_cred(&minor_status, server_name, GSS_C_INDEFINITE,
                             GSS_C_NO_OID_SET, GSS_C_ACCEPT, &server_creds, NULL, NULL);
        if (check_gss_err(major_status, minor_status, "gss_acquire_cred()", log, 1))
            goto cleanup;

        major_status = gss_accept_sec_context(&minor_status,
                                              &gss_context,
                                              server_creds,
                                              &input_token,
                                              GSS_C_NO_CHANNEL_BINDINGS,
                                              &client_name, NULL, &output_token, &ret_flags, NULL, NULL);

        if (output_token.length) {
            spnegoToken = (const unsigned char *) output_token.value;
            spnegoTokenLength = output_token.length;
            token = (char *) xmalloc((size_t)base64_encode_len((int)spnegoTokenLength));
            if (token == NULL) {
                debug((char *) "%s| %s: ERROR: Not enough memory\n", LogTime(), PROGRAM);
                fprintf(stdout, "BH Not enough memory\n");
                goto cleanup;
            }
            base64_encode_str(token, base64_encode_len((int)spnegoTokenLength),
                              (const char *) spnegoToken, (int)spnegoTokenLength);

            if (check_gss_err(major_status, minor_status, "gss_accept_sec_context()", log, 1))
                goto cleanup;
            if (major_status & GSS_S_CONTINUE_NEEDED) {
                debug((char *) "%s| %s: INFO: continuation needed\n", LogTime(), PROGRAM);
                fprintf(stdout, "TT %s\n", token);
                goto cleanup;
            }
            gss_release_buffer(&minor_status, &output_token);
            major_status =
                gss_display_name(&minor_status, client_name, &output_token,
                                 NULL);

            if (check_gss_err(major_status, minor_status, "gss_display_name()", log, 1))
                goto cleanup;
            user = (char *) xmalloc(output_token.length + 1);
            if (user == NULL) {
                debug((char *) "%s| %s: ERROR: Not enough memory\n", LogTime(), PROGRAM);
                fprintf(stdout, "BH Not enough memory\n");
                goto cleanup;
            }
            memcpy(user, output_token.value, output_token.length);
            user[output_token.length] = '\0';
            if (norealm && (p = strchr(user, '@')) != NULL) {
                *p = '\0';
            }

#if HAVE_PAC_SUPPORT
            ret = krb5_init_context(&context);
            if (!check_k5_err(context, "krb5_init_context", ret)) {
#if HAVE_HEIMDAL_KERBEROS
#define ADWIN2KPAC 128
                major_status = gsskrb5_extract_authz_data_from_sec_context(&minor_status,
                               gss_context, ADWIN2KPAC, &data_set);
                if (!check_gss_err(major_status, minor_status,
                                   "gsskrb5_extract_authz_data_from_sec_context()", log, 0)) {
                    ret = krb5_pac_parse(context, data_set.value, data_set.length, &pac);
                    gss_release_buffer(&minor_status, &data_set);
                    if (!check_k5_err(context, "krb5_pac_parse", ret)) {
                        ag = get_ad_groups((char *)&ad_groups, context, pac);
                        krb5_pac_free(context, pac);
                    }
                    krb5_free_context(context);
                }
#else
                type_id.value = (void *)"mspac";
                type_id.length = strlen((char *)type_id.value);
#define KRB5PACLOGONINFO        1
                major_status = gss_map_name_to_any(&minor_status, client_name, KRB5PACLOGONINFO, &type_id, (gss_any_t *)&pac);
                if (!check_gss_err(major_status, minor_status, "gss_map_name_to_any()", log, 0)) {
                    ag = get_ad_groups((char *)&ad_groups,context, pac);
                }
                (void)gss_release_any_name_mapping(&minor_status, client_name, &type_id, (gss_any_t *)&pac);
                krb5_free_context(context);
#endif
            }
            if (ag) {
                debug((char *) "%s| %s: DEBUG: Groups %s\n", LogTime(), PROGRAM, ag);
            }
#endif
            fprintf(stdout, "AF %s %s\n", token, user);
            rfc_user = rfc1738_escape(user);
            debug((char *) "%s| %s: DEBUG: AF %s %s\n", LogTime(), PROGRAM, token, rfc_user);
            if (log)
                fprintf(stderr, "%s| %s: INFO: User %s authenticated\n", LogTime(),
                        PROGRAM, rfc1738_escape(user));
            goto cleanup;
        } else {
            if (check_gss_err(major_status, minor_status, "gss_accept_sec_context()", log, 1))
                goto cleanup;
            if (major_status & GSS_S_CONTINUE_NEEDED) {
                debug((char *) "%s| %s: INFO: continuation needed\n", LogTime(), PROGRAM);
                fprintf(stdout, "NA %s\n", token);
                goto cleanup;
            }
            gss_release_buffer(&minor_status, &output_token);
            major_status =
                gss_display_name(&minor_status, client_name, &output_token,
                                 NULL);

            if (check_gss_err(major_status, minor_status, "gss_display_name()", log, 1))
                goto cleanup;
            /*
             *  Return dummy token AA. May need an extra return tag then AF
             */
            user = (char *) xmalloc(output_token.length + 1);
            if (user == NULL) {
                debug((char *) "%s| %s: ERROR: Not enough memory\n", LogTime(), PROGRAM);
                fprintf(stdout, "BH Not enough memory\n");
                goto cleanup;
            }
            memcpy(user, output_token.value, output_token.length);
            user[output_token.length] = '\0';
            if (norealm && (p = strchr(user, '@')) != NULL) {
                *p = '\0';
            }
            fprintf(stdout, "AF %s %s\n", "AA==", user);
            debug((char *) "%s| %s: DEBUG: AF %s %s\n", LogTime(), PROGRAM, "AA==", rfc1738_escape(user));
            if (log)
                fprintf(stderr, "%s| %s: INFO: User %s authenticated\n", LogTime(),
                        PROGRAM, rfc1738_escape(user));

        }
cleanup:
        gss_release_buffer(&minor_status, &input_token);
        gss_release_buffer(&minor_status, &output_token);
        gss_release_cred(&minor_status, &server_creds);
        if (server_name)
            gss_release_name(&minor_status, &server_name);
        if (client_name)
            gss_release_name(&minor_status, &client_name);
        if (kerberosToken) {
            /* Allocated by parseNegTokenInit, but no matching free function exists.. */
            if (!spnego_flag)
                safe_free(kerberosToken);
        }
        if (spnego_flag) {
            /* Allocated by makeNegTokenTarg, but no matching free function exists.. */
            safe_free(spnegoToken);
        }
        safe_free(token);
        safe_free(user);
        continue;
    }
}