        DosFreeMem(parent_info);

        /* Do the work */
        ap_mpm_child_main(pconf);

        /* Outta here */
        return 1;
    }
    else {
        /* Parent process */
        char restart;
        is_parent_process = TRUE;

        if (ap_setup_listeners(ap_server_conf) < 1) {
            ap_log_error(APLOG_MARK, APLOG_ALERT, 0, s, APLOGNO(00200)
                         "no listening sockets available, shutting down");
            return 1;
        }

        ap_log_pid(pconf, ap_pid_fname);

        restart = master_main();
        ++ap_my_generation;
        ap_scoreboard_image->global->running_generation = ap_my_generation;

        if (!restart) {
            ap_remove_pid(pconf, ap_pid_fname);
            ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, ap_server_conf, APLOGNO(00201)
                         "caught SIGTERM, shutting down");
            return 1;
        }
    }  /* Parent process */

    return 0; /* Restart */
}



/* Main processing of the parent process
 * returns TRUE if restarting
 */
static char master_main()
{
    server_rec *s = ap_server_conf;
    ap_listen_rec *lr;
    parent_info_t *parent_info;
    char *listener_shm_name;
    int listener_num, num_listeners, slot;
