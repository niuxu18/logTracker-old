			ret = ARCHIVE_OK;
		}
		return (ret);
	} else if (strcmp(key, "compression-level") == 0) {
		if (val == NULL || !(val[0] >= '0' && val[0] <= '9') || val[1] != '\0') {
			return ARCHIVE_WARN;
		}

		if (val[0] == '0') {
			zip->requested_compression = COMPRESSION_STORE;
			return ARCHIVE_OK;
		} else {
#ifdef HAVE_ZLIB_H
			zip->requested_compression = COMPRESSION_DEFLATE;
			zip->deflate_compression_level = val[0] - '0';
			return ARCHIVE_OK;
#else
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "deflate compression not supported");
#endif
		}
	} else if (strcmp(key, "encryption") == 0) {
		if (val == NULL) {
			zip->encryption_type = ENCRYPTION_NONE;
			ret = ARCHIVE_OK;
		} else if (val[0] == '1' || strcmp(val, "traditional") == 0
		    || strcmp(val, "zipcrypt") == 0
		    || strcmp(val, "ZipCrypt") == 0) {
			if (is_traditional_pkware_encryption_supported()) {
				zip->encryption_type = ENCRYPTION_TRADITIONAL;
				ret = ARCHIVE_OK;
			} else {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "encryption not supported");
			}
		} else if (strcmp(val, "aes128") == 0) {
			if (is_winzip_aes_encryption_supported(
			    ENCRYPTION_WINZIP_AES128)) {
				zip->encryption_type = ENCRYPTION_WINZIP_AES128;
				ret = ARCHIVE_OK;
			} else {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "encryption not supported");
			}
		} else if (strcmp(val, "aes256") == 0) {
			if (is_winzip_aes_encryption_supported(
			    ENCRYPTION_WINZIP_AES256)) {
				zip->encryption_type = ENCRYPTION_WINZIP_AES256;
				ret = ARCHIVE_OK;
			} else {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "encryption not supported");
			}
		} else {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "%s: unknown encryption '%s'",
			    a->format_name, val);
		}
		return (ret);
	} else if (strcmp(key, "experimental") == 0) {
		if (val == NULL || val[0] == 0) {
			zip->flags &= ~ ZIP_FLAG_EXPERIMENT_xl;
		} else {
			zip->flags |= ZIP_FLAG_EXPERIMENT_xl;
		}
		return (ARCHIVE_OK);
	} else if (strcmp(key, "fakecrc32") == 0) {
