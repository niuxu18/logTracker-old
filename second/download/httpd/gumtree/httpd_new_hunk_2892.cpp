                    frag = req_dat->tail;
                    break;
                }
                else {
                    /* should not be possible */
                    /* abandon malfunctioning module */
                    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01509)
                                MODNAME ": bad state %d (ws)", state);
                    return DECLINED;
                }
                /* NOTREACHED */
            }
            else if (state == rsl_type &&
