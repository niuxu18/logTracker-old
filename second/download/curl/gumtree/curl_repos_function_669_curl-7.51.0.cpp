static ssize_t write_behind(struct testcase *test, int convert)
{
  char *writebuf;
  int count;
  int ct;
  char *p;
  int c;                          /* current character */
  struct bf *b;
  struct tftphdr *dp;

  b = &bfs[nextone];
  if(b->counter < -1)            /* anything to flush? */
    return 0;                     /* just nop if nothing to do */

  if(!test->ofile) {
    char outfile[256];
    snprintf(outfile, sizeof(outfile), "log/upload.%ld", test->testno);
#ifdef WIN32
    test->ofile=open(outfile, O_CREAT|O_RDWR|O_BINARY, 0777);
#else
    test->ofile=open(outfile, O_CREAT|O_RDWR, 0777);
#endif
    if(test->ofile == -1) {
      logmsg("Couldn't create and/or open file %s for upload!", outfile);
      return -1; /* failure! */
    }
  }

  count = b->counter;             /* remember byte count */
  b->counter = BF_FREE;           /* reset flag */
  dp = &b->buf.hdr;
  nextone = !nextone;             /* incr for next time */
  writebuf = dp->th_data;

  if(count <= 0)
    return -1;                    /* nak logic? */

  if(convert == 0)
    return write(test->ofile, writebuf, count);

  p = writebuf;
  ct = count;
  while(ct--) {                   /* loop over the buffer */
    c = *p++;                     /* pick up a character */
    if(prevchar == '\r') {        /* if prev char was cr */
      if(c == '\n')               /* if have cr,lf then just */
        lseek(test->ofile, -1, SEEK_CUR); /* smash lf on top of the cr */
      else
        if(c == '\0')             /* if have cr,nul then */
          goto skipit;            /* just skip over the putc */
      /* else just fall through and allow it */
    }
    /* formerly
       putc(c, file); */
    if(1 != write(test->ofile, &c, 1))
      break;
    skipit:
    prevchar = c;
  }
  return count;
}