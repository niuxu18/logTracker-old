(p + 8 > endp) {
      archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
        "Invalid header size");
      return (ARCHIVE_FATAL);
    }