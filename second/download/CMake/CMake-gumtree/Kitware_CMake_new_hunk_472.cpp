      rar->range_dec.Stream = &rar->bytein;
      __archive_ppmd7_functions.Ppmd7_Construct(&rar->ppmd7_context);

      if (rar->dictionary_size == 0) {
	      archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
                          "Invalid zero dictionary size");
	      return (ARCHIVE_FATAL);
      }

      if (!__archive_ppmd7_functions.Ppmd7_Alloc(&rar->ppmd7_context,
        rar->dictionary_size, &g_szalloc))
      {
