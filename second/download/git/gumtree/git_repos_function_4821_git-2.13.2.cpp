int SHA1DCFinal(unsigned char output[20], SHA1_CTX *ctx)
{
	uint32_t last = ctx->total & 63;
	uint32_t padn = (last < 56) ? (56 - last) : (120 - last);
	uint64_t total;
	SHA1DCUpdate(ctx, (const char*)(sha1_padding), padn);

	total = ctx->total - padn;
	total <<= 3;
	ctx->buffer[56] = (unsigned char)(total >> 56);
	ctx->buffer[57] = (unsigned char)(total >> 48);
	ctx->buffer[58] = (unsigned char)(total >> 40);
	ctx->buffer[59] = (unsigned char)(total >> 32);
	ctx->buffer[60] = (unsigned char)(total >> 24);
	ctx->buffer[61] = (unsigned char)(total >> 16);
	ctx->buffer[62] = (unsigned char)(total >> 8);
	ctx->buffer[63] = (unsigned char)(total);
	sha1_process(ctx, (uint32_t*)(ctx->buffer));
	output[0] = (unsigned char)(ctx->ihv[0] >> 24);
	output[1] = (unsigned char)(ctx->ihv[0] >> 16);
	output[2] = (unsigned char)(ctx->ihv[0] >> 8);
	output[3] = (unsigned char)(ctx->ihv[0]);
	output[4] = (unsigned char)(ctx->ihv[1] >> 24);
	output[5] = (unsigned char)(ctx->ihv[1] >> 16);
	output[6] = (unsigned char)(ctx->ihv[1] >> 8);
	output[7] = (unsigned char)(ctx->ihv[1]);
	output[8] = (unsigned char)(ctx->ihv[2] >> 24);
	output[9] = (unsigned char)(ctx->ihv[2] >> 16);
	output[10] = (unsigned char)(ctx->ihv[2] >> 8);
	output[11] = (unsigned char)(ctx->ihv[2]);
	output[12] = (unsigned char)(ctx->ihv[3] >> 24);
	output[13] = (unsigned char)(ctx->ihv[3] >> 16);
	output[14] = (unsigned char)(ctx->ihv[3] >> 8);
	output[15] = (unsigned char)(ctx->ihv[3]);
	output[16] = (unsigned char)(ctx->ihv[4] >> 24);
	output[17] = (unsigned char)(ctx->ihv[4] >> 16);
	output[18] = (unsigned char)(ctx->ihv[4] >> 8);
	output[19] = (unsigned char)(ctx->ihv[4]);
	return ctx->found_collision;
}