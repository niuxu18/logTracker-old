         add_password(user, realm, tfp);
     }
     apr_file_close(f);
 
     /* The temporary file has all the data, just copy it to the new location.
      */
-    if (apr_file_copy(dirname, argv[1], APR_FILE_SOURCE_PERMS, cntxt) !=
+    if (apr_file_copy(dirname, argv[1], APR_OS_DEFAULT, cntxt) !=
                 APR_SUCCESS) {
         apr_file_printf(errfile, "%s: unable to update file %s\n",
                         argv[0], argv[1]);
     }
     apr_file_close(tfp);
 
