{
    archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
                      "Prefix found");
    return (ARCHIVE_FATAL);
  }