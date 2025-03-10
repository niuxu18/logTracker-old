static void my_lock(CURL *handle, curl_lock_data data, curl_lock_access laccess,
          void *useptr )
{
  const char *what;
  struct userdata *user = (struct userdata *)useptr;
  int locknum;

  (void)handle;
  (void)laccess;

  switch ( data ) {
    case CURL_LOCK_DATA_SHARE:
      what = "share";
      locknum = 0;
      break;
    case CURL_LOCK_DATA_DNS:
      what = "dns";
      locknum = 1;
      break;
    case CURL_LOCK_DATA_COOKIE:
      what = "cookie";
      locknum = 2;
      break;
    default:
      fprintf(stderr, "lock: no such data: %d\n", (int)data);
      return;
  }

  /* detect locking of locked locks */
  if(lock[locknum]) {
    printf("lock: double locked %s\n", what);
    return;
  }
  lock[locknum]++;

  printf("lock:   %-6s [%s]: %d\n", what, user->text, user->counter);
  user->counter++;
}