{
	static const uint8_t dict_pow2[]
			= { 18, 20, 21, 22, 22, 23, 23, 24, 25, 26 };
	static const uint8_t depths[] = { 4, 8, 24, 48 };

	const uint32_t level = preset & LZMA_PRESET_LEVEL_MASK;
	const uint32_t flags = preset & ~LZMA_PRESET_LEVEL_MASK;
	const uint32_t supported_flags = LZMA_PRESET_EXTREME;

	if (level > 9 || (flags & ~supported_flags))
		return true;

	options->preset_dict = NULL;
	options->preset_dict_size = 0;

	options->lc = LZMA_LC_DEFAULT;
	options->lp = LZMA_LP_DEFAULT;
	options->pb = LZMA_PB_DEFAULT;

	options->dict_size = UINT32_C(1) << dict_pow2[level];

	if (level <= 3) {
		options->mode = LZMA_MODE_FAST;
		options->mf = level == 0 ? LZMA_MF_HC3 : LZMA_MF_HC4;
		options->nice_len = level <= 1 ? 128 : 273;
		options->depth = depths[level];
	} else {
		options->mode = LZMA_MODE_NORMAL;
		options->mf = LZMA_MF_BT4;
		options->nice_len = level == 4 ? 16 : level == 5 ? 32 : 64;
		options->depth = 0;
	}

	if (flags & LZMA_PRESET_EXTREME) {
		options->mode = LZMA_MODE_NORMAL;
		options->mf = LZMA_MF_BT4;
		if (level == 3 || level == 5) {
			options->nice_len = 192;
			options->depth = 0;
		} else {
			options->nice_len = 273;
			options->depth = 512;
		}
	}

	return false;
}