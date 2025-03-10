void SHA256_Transform(SHA256_CTX* context, const sha2_word32* data) {
        sha2_word32     a, b, c, d, e, f, g, h, s0, s1;
        sha2_word32     T1, T2, *W256;
        int             j;

        W256 = (sha2_word32*)context->buffer;

        /* Initialize registers with the prev. intermediate value */
        a = context->state[0];
        b = context->state[1];
        c = context->state[2];
        d = context->state[3];
        e = context->state[4];
        f = context->state[5];
        g = context->state[6];
        h = context->state[7];

        j = 0;
        do {
#if !APR_IS_BIGENDIAN
                /* Copy data while converting to host byte order */
                REVERSE32(*data++,W256[j]);
                /* Apply the SHA-256 compression function to update a..h */
                T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + W256[j];
#else /* APR_IS_BIGENDIAN */
                /* Apply the SHA-256 compression function to update a..h with copy */
                T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + (W256[j] = *data++);
#endif /* APR_IS_BIGENDIAN */
                T2 = Sigma0_256(a) + Maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;

                j++;
        } while (j < 16);

        do {
                /* Part of the message block expansion: */
                s0 = W256[(j+1)&0x0f];
                s0 = sigma0_256(s0);
                s1 = W256[(j+14)&0x0f]; 
                s1 = sigma1_256(s1);

                /* Apply the SHA-256 compression function to update a..h */
                T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + 
                     (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0);
                T2 = Sigma0_256(a) + Maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;

                j++;
        } while (j < 64);

        /* Compute the current intermediate hash value */
        context->state[0] += a;
        context->state[1] += b;
        context->state[2] += c;
        context->state[3] += d;
        context->state[4] += e;
        context->state[5] += f;
        context->state[6] += g;
        context->state[7] += h;

        /* Clean up */
        a = b = c = d = e = f = g = h = T1 = T2 = 0;
}