           retry_sleep = retry_sleep_default;
           break;
 
         } while(1);
 
         if((config->progressmode == CURL_PROGRESS_BAR) &&
-           progressbar.calls) {
+           progressbar.calls)
           /* if the custom progress bar has been displayed, we output a
              newline here */
           fputs("\n", progressbar.out);
-        }
 
-        if(config->writeout) {
+        if(config->writeout)
           ourWriteOut(curl, config->writeout);
-        }
 #ifdef USE_ENVIRONMENT
         if (config->writeenv)
           ourWriteEnv(curl);
 #endif
 
 show_error:
