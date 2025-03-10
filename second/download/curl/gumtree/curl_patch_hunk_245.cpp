       break;
     case 'r':
       /* Specifying a range WITHOUT A DASH will create an illegal HTTP range
          (and won't actually be range by definition). The man page previously
          claimed that to be a good way, why this code is added to work-around
          it. */
-      if(!strchr(nextarg, '-')) {
+      if(ISDIGIT(*nextarg) && !strchr(nextarg, '-')) {
         char buffer[32];
         curl_off_t off;
         warnf(config,
               "A specfied range MUST include at least one dash (-). "
               "Appending one for you!\n");
         off = curlx_strtoofft(nextarg, NULL, 10);
         snprintf(buffer, sizeof(buffer), "%Od-", off);
         GetStr(&config->range, buffer);
       }
-      else
+      {
+        /* byte range requested */
+        char* tmp_range;
+        tmp_range=nextarg;
+        while(*tmp_range != '\0') {
+          if(!ISDIGIT(*tmp_range)&&*tmp_range!='-'&&*tmp_range!=',') {
+            warnf(config,"Invalid character is found in given range. "
+                  "A specified range MUST have only digits in "
+                  "\'start\'-\'stop\'. The server's response to this "
+                  "request is uncertain.\n");
+            break;
+          }
+          tmp_range++;
+        }
         /* byte range requested */
         GetStr(&config->range, nextarg);
-
+      }
       break;
     case 'R':
       /* use remote file's time */
       config->remote_time ^= TRUE;
       break;
     case 's':
       /* don't show progress meter, don't show errors : */
-      config->conf |= (CONF_MUTE|CONF_NOPROGRESS);
+      config->conf ^= (CONF_MUTE|CONF_NOPROGRESS);
       config->showerror ^= TRUE; /* toggle off */
       break;
     case 'S':
       /* show errors */
       config->showerror ^= TRUE; /* toggle on if used with -s */
       break;
