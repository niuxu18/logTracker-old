(node < 0 || node >= code->numentries)
  {
    archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
                      "Invalid location to Huffman tree specified.");
    return (ARCHIVE_FATAL);
  }