        ** no more urls left.
        */

        quit_urls:

        /* Set file extended attributes */
        if(!result && config->xattr && outs.fopened && outs.stream) {
          int rc = fwrite_xattr(curl, fileno(outs.stream));
          if(rc)
            warnf(config, "Error setting extended attributes: %s\n",
                  strerror(errno));
        }

        /* Close the file */
        if(outs.fopened && outs.stream) {
          int rc = fclose(outs.stream);
          if(!result && rc) {
            /* something went wrong in the writing process */
            result = CURLE_WRITE_ERROR;
            fprintf(global->errors, "(%d) Failed writing body\n", result);
          }
        }
        else if(!outs.s_isreg && outs.stream) {
          /* Dump standard stream buffered data */
          int rc = fflush(outs.stream);
          if(!result && rc) {
            /* something went wrong in the writing process */
            result = CURLE_WRITE_ERROR;
            fprintf(global->errors, "(%d) Failed writing body\n", result);
          }
        }

#ifdef __AMIGA__
        if(!result && outs.s_isreg && outs.filename) {
          /* Set the url (up to 80 chars) as comment for the file */
          if(strlen(url) > 78)
            url[79] = '\0';
          SetComment(outs.filename, url);
        }
#endif

#ifdef HAVE_UTIME
        /* File time can only be set _after_ the file has been closed */
        if(!result && config->remote_time && outs.s_isreg && outs.filename) {
          /* Ask libcurl if we got a remote file time */
          long filetime = -1;
          curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
          if(filetime >= 0) {
            struct utimbuf times;
            times.actime = (time_t)filetime;
