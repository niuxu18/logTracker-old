  printf("  ==> extracting: %s (character device %ld,%ld)\n",
         filename, devmaj, devmin);
#endif
#if !defined(WIN32) && !defined(__VMS)
  if (mknod(filename, mode | S_IFCHR,
      compat_makedev(devmaj, devmin)) == -1)
#else
