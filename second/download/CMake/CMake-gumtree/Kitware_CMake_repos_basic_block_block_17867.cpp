{
			crc = lzma_crc64_table[0][*buf++ ^ A1(crc)] ^ S8(crc);
			--size;
		}