{
    len = snprintf(ptr, left, " libidn2/%s", idn2_check_version(NULL));
    left -= len;
    ptr += len;
  }