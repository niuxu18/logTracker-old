            apr_md5_encode((const char *)htdbm->userpass, (const char *)salt,
                            cpw, sizeof(cpw));
        break;
        case ALG_PLAIN:
            /* XXX this len limitation is not in sync with any HTTPd len. */
            apr_cpystrn(cpw,htdbm->userpass,sizeof(cpw));
#if (!(defined(WIN32) || defined(TPF) || defined(NETWARE)))
            fprintf(stderr, "Warning: Plain text passwords aren't supported by the "
                    "server on this platform!\n");
#endif
        break;
#if (!(defined(WIN32) || defined(TPF) || defined(NETWARE)))
        case ALG_CRYPT:
            (void) srand((int) time((time_t *) NULL));
            to64(&salt[0], rand(), 8);
            salt[8] = '\0';
            cbuf = crypt(htdbm->userpass, salt);
            if (cbuf == NULL) {
                char errbuf[128];
                
                fprintf(stderr, "crypt() failed: %s\n", 
                        apr_strerror(errno, errbuf, sizeof errbuf));
                exit(ERR_PWMISMATCH);
            }
            apr_cpystrn(cpw, cbuf, sizeof(cpw) - 1);
            fprintf(stderr, "CRYPT is now deprecated, use MD5 instead!\n");
#endif
        default:
        break;
    }
    htdbm->userpass = apr_pstrdup(htdbm->pool, cpw);
