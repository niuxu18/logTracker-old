        break;
      }
      if(err)
        return err;
      break;
    case 'r':
      /* Specifying a range WITHOUT A DASH will create an illegal HTTP range
         (and won't actually be range by definition). The man page previously
         claimed that to be a good way, why this code is added to work-around
         it. */
      if(!strchr(nextarg, '-')) {
        char buffer[32];
        curl_off_t off;
        warnf(config,
              "A specfied range MUST include at least one dash (-). "
              "Appending one for you!\n");
        off = curlx_strtoofft(nextarg, NULL, 10);
        snprintf(buffer, sizeof(buffer), CURL_FORMAT_OFF_T "-", off);
        GetStr(&config->range, buffer);
      }
      else
        /* byte range requested */
        GetStr(&config->range, nextarg);

      break;
    case 'R':
      /* use remote file's time */
      config->remote_time ^= TRUE;
      break;
    case 's':
