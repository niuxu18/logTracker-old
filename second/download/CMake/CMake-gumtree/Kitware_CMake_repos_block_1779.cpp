{
	printf("/* This file has been automatically generated by "
			"crc64_tablegen.c. */\n\n"
			"const uint64_t lzma_crc64_table[4][256] = {\n\t{");

	for (size_t s = 0; s < 4; ++s) {
		for (size_t b = 0; b < 256; ++b) {
			if ((b % 2) == 0)
				printf("\n\t\t");

			printf("UINT64_C(0x%016" PRIX64 ")",
					crc64_table[s][b]);

			if (b != 255)
				printf(",%s", (b+1) % 2 == 0 ? "" : " ");
		}

		if (s == 3)
			printf("\n\t}\n};\n");
		else
			printf("\n\t}, {");
	}

	return;
}