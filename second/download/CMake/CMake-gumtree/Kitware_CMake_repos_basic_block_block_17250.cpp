{
		if (!v7tar->init_default_conversion) {
			v7tar->sconv_default =
			    archive_string_default_conversion_for_write(
				&(a->archive));
			v7tar->init_default_conversion = 1;
		}
		sconv = v7tar->sconv_default;
	}