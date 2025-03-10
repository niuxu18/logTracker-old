                         * of which would make the parent shutdown all
                         * children, then idle-loop until it detected that
                         * the network is up again, and restart the children.
                         * Ben Hyde noted that temporary ENETDOWN situations
                         * occur in mobile IP.
                         */
-                        ap_log_error(APLOG_MARK, APLOG_EMERG, stat, ap_server_conf,
+                        ap_log_error(APLOG_MARK, APLOG_EMERG, stat, ap_server_conf, APLOGNO(00218)
                             "apr_socket_accept: giving up.");
                         clean_child_exit(APEXIT_CHILDFATAL, my_worker_num, ptrans,
                                          bucket_alloc);
                 }
 #endif
                 else {
-                        ap_log_error(APLOG_MARK, APLOG_ERR, stat, ap_server_conf,
+                        ap_log_error(APLOG_MARK, APLOG_ERR, stat, ap_server_conf, APLOGNO(00219)
                             "apr_socket_accept: (client socket)");
                         clean_child_exit(1, my_worker_num, ptrans, bucket_alloc);
                 }
             }
         }
 
