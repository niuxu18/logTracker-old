  libtar_hashptr_t hp;
  tar_dev_t *td = NULL;
  tar_ino_t *ti = NULL;
  char path[MAXPATHLEN];

#ifdef DEBUG
  printf("==> tar_append_file(TAR=0x%lx (\"%s\"), realname=\"%s\", "
