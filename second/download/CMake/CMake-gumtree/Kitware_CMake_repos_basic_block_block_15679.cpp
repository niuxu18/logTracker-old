{
			cpio->sconv_default =
			    archive_string_default_conversion_for_write(
			      &(a->archive));
			cpio->init_default_conversion = 1;
		}