   incoming bit, x^32 is added mod p to the register if the bit is a one (where
   x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by
   x (which is shifting right by one and adding x^32 mod p if the bit shifted
   out is a one).  We start with the highest power (least significant bit) of
   q and repeat for all eight bits of q.
 
-  The table is simply the CRC of all possible eight bit values.  This is all
-  the information needed to generate CRC's on data a byte at a time for all
-  combinations of CRC register values and incoming bytes.
+  The first table is simply the CRC of all possible eight bit values.  This is
+  all the information needed to generate CRCs on data a byte at a time for all
+  combinations of CRC register values and incoming bytes.  The remaining tables
+  allow for word-at-a-time CRC calculation for both big-endian and little-
+  endian machines, where a word is four bytes.
 */
 local void make_crc_table()
 {
-  uLong c;
-  int n, k;
-  uLong poly;            /* polynomial exclusive-or pattern */
-  /* terms of polynomial defining this crc (except x^32): */
-  static const Byte p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};
-
-  /* make exclusive-or pattern from polynomial (0xedb88320L) */
-  poly = 0L;
-  for (n = 0; n < sizeof(p)/sizeof(Byte); n++)
-    poly |= 1L << (31 - p[n]);
- 
-  for (n = 0; n < 256; n++)
-  {
-    c = (uLong)n;
-    for (k = 0; k < 8; k++)
-      c = c & 1 ? poly ^ (c >> 1) : c >> 1;
-    crc_table[n] = c;
-  }
-  crc_table_empty = 0;
+    unsigned long c;
+    int n, k;
+    unsigned long poly;                 /* polynomial exclusive-or pattern */
+    /* terms of polynomial defining this crc (except x^32): */
+    static volatile int first = 1;      /* flag to limit concurrent making */
+    static const unsigned char p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};
+
+    /* See if another task is already doing this (not thread-safe, but better
+       than nothing -- significantly reduces duration of vulnerability in
+       case the advice about DYNAMIC_CRC_TABLE is ignored) */
+    if (first) {
+        first = 0;
+
+        /* make exclusive-or pattern from polynomial (0xedb88320UL) */
+        poly = 0UL;
+        for (n = 0; n < sizeof(p)/sizeof(unsigned char); n++)
+            poly |= 1UL << (31 - p[n]);
+
+        /* generate a crc for every 8-bit value */
+        for (n = 0; n < 256; n++) {
+            c = (unsigned long)n;
+            for (k = 0; k < 8; k++)
+                c = c & 1 ? poly ^ (c >> 1) : c >> 1;
+            crc_table[0][n] = c;
+        }
+
+#ifdef BYFOUR
+        /* generate crc for each value followed by one, two, and three zeros,
+           and then the byte reversal of those as well as the first table */
+        for (n = 0; n < 256; n++) {
+            c = crc_table[0][n];
+            crc_table[4][n] = REV(c);
+            for (k = 1; k < 4; k++) {
+                c = crc_table[0][c & 0xff] ^ (c >> 8);
+                crc_table[k][n] = c;
+                crc_table[k + 4][n] = REV(c);
+            }
+        }
+#endif /* BYFOUR */
+
+        crc_table_empty = 0;
+    }
+    else {      /* not first */
+        /* wait for the other guy to finish (not efficient, but rare) */
+        while (crc_table_empty)
+            ;
+    }
+
+#ifdef MAKECRCH
+    /* write out CRC tables to crc32.h */
+    {
+        FILE *out;
+
+        out = fopen("crc32.h", "w");
+        if (out == NULL) return;
+        fprintf(out, "/* crc32.h -- tables for rapid CRC calculation\n");
+        fprintf(out, " * Generated automatically by crc32.c\n */\n\n");
+        fprintf(out, "local const unsigned long FAR ");
+        fprintf(out, "crc_table[TBLS][256] =\n{\n  {\n");
+        write_table(out, crc_table[0]);
+#  ifdef BYFOUR
+        fprintf(out, "#ifdef BYFOUR\n");
+        for (k = 1; k < 8; k++) {
+            fprintf(out, "  },\n  {\n");
+            write_table(out, crc_table[k]);
+        }
+        fprintf(out, "#endif\n");
+#  endif /* BYFOUR */
+        fprintf(out, "  }\n};\n");
+        fclose(out);
+    }
+#endif /* MAKECRCH */
 }
-#else
+
+#ifdef MAKECRCH
+local void write_table(out, table)
+    FILE *out;
+    const unsigned long FAR *table;
+{
+    int n;
+
+    for (n = 0; n < 256; n++)
+        fprintf(out, "%s0x%08lxUL%s", n % 5 ? "" : "    ", table[n],
+                n == 255 ? "\n" : (n % 5 == 4 ? ",\n" : ", "));
+}
+#endif /* MAKECRCH */
+
+#else /* !DYNAMIC_CRC_TABLE */
 /* ========================================================================
- * Table of CRC-32's of all single-byte values (made by make_crc_table)
+ * Tables of CRC-32s of all single-byte values, made by make_crc_table().
  */
-local const uLongf crc_table[256] = {
-  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
-  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
-  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
-  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
-  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
-  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
-  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
-  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
-  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
-  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
-  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
-  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
-  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
-  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
-  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
-  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
-  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
-  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
-  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
-  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
-  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
-  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
-  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
-  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
-  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
-  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
-  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
-  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
-  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
-  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
-  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
-  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
-  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
-  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
-  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
-  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
-  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
-  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
-  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
-  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
-  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
-  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
-  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
-  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
-  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
-  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
-  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
-  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
-  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
-  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
-  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
-  0x2d02ef8dL
-};
-#endif
+#include "crc32.h"
+#endif /* DYNAMIC_CRC_TABLE */
 
 /* =========================================================================
  * This function can be used by asm versions of crc32()
  */
-const uLongf * ZEXPORT get_crc_table()
+const unsigned long FAR * ZEXPORT get_crc_table()
 {
 #ifdef DYNAMIC_CRC_TABLE
-  if (crc_table_empty) make_crc_table();
-#endif
-  return (const uLongf *)crc_table;
+    if (crc_table_empty)
+        make_crc_table();
+#endif /* DYNAMIC_CRC_TABLE */
+    return (const unsigned long FAR *)crc_table;
 }
 
 /* ========================================================================= */
-#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
-#define DO2(buf)  DO1(buf); DO1(buf);
-#define DO4(buf)  DO2(buf); DO2(buf);
-#define DO8(buf)  DO4(buf); DO4(buf);
+#define DO1 crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8)
+#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1
 
 /* ========================================================================= */
-uLong ZEXPORT crc32(crc, buf, len)
-    uLong crc;
-    const Bytef *buf;
-    uInt len;
+unsigned long ZEXPORT crc32(crc, buf, len)
+    unsigned long crc;
+    const unsigned char FAR *buf;
+    unsigned len;
 {
-    if (buf == Z_NULL) return 0L;
+    if (buf == Z_NULL) return 0UL;
+
 #ifdef DYNAMIC_CRC_TABLE
     if (crc_table_empty)
-      make_crc_table();
-#endif
-    crc = crc ^ 0xffffffffL;
-    while (len >= 8)
-    {
-      DO8(buf);
-      len -= 8;
+        make_crc_table();
+#endif /* DYNAMIC_CRC_TABLE */
+
+#ifdef BYFOUR
+    if (sizeof(void *) == sizeof(ptrdiff_t)) {
+        u4 endian;
+
+        endian = 1;
+        if (*((unsigned char *)(&endian)))
+            return crc32_little(crc, buf, len);
+        else
+            return crc32_big(crc, buf, len);
+    }
+#endif /* BYFOUR */
+    crc = crc ^ 0xffffffffUL;
+    while (len >= 8) {
+        DO8;
+        len -= 8;
+    }
+    if (len) do {
+        DO1;
+    } while (--len);
+    return crc ^ 0xffffffffUL;
+}
+
+#ifdef BYFOUR
+
+/* ========================================================================= */
+#define DOLIT4 c ^= *buf4++; \
+        c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ \
+            crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24]
+#define DOLIT32 DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4
+
+/* ========================================================================= */
+local unsigned long crc32_little(crc, buf, len)
+    unsigned long crc;
+    const unsigned char FAR *buf;
+    unsigned len;
+{
+    register u4 c;
+    register const u4 FAR *buf4;
+
+    c = (u4)crc;
+    c = ~c;
+    while (len && ((ptrdiff_t)buf & 3)) {
+        c = crc_table[0][(c ^ *buf++) & 0xff] ^ (c >> 8);
+        len--;
+    }
+
+    buf4 = (const u4 FAR *)(const void FAR *)buf;
+    while (len >= 32) {
+        DOLIT32;
+        len -= 32;
+    }
+    while (len >= 4) {
+        DOLIT4;
+        len -= 4;
     }
+    buf = (const unsigned char FAR *)buf4;
+
     if (len) do {
-      DO1(buf);
+        c = crc_table[0][(c ^ *buf++) & 0xff] ^ (c >> 8);
     } while (--len);
-    return crc ^ 0xffffffffL;
+    c = ~c;
+    return (unsigned long)c;
+}
+
+/* ========================================================================= */
+#define DOBIG4 c ^= *++buf4; \
+        c = crc_table[4][c & 0xff] ^ crc_table[5][(c >> 8) & 0xff] ^ \
+            crc_table[6][(c >> 16) & 0xff] ^ crc_table[7][c >> 24]
+#define DOBIG32 DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4
+
+/* ========================================================================= */
+local unsigned long crc32_big(crc, buf, len)
+    unsigned long crc;
+    const unsigned char FAR *buf;
+    unsigned len;
+{
+    register u4 c;
+    register const u4 FAR *buf4;
+
+    c = REV((u4)crc);
+    c = ~c;
+    while (len && ((ptrdiff_t)buf & 3)) {
+        c = crc_table[4][(c >> 24) ^ *buf++] ^ (c << 8);
+        len--;
+    }
+
+    buf4 = (const u4 FAR *)(const void FAR *)buf;
+    buf4--;
+    while (len >= 32) {
+        DOBIG32;
+        len -= 32;
+    }
+    while (len >= 4) {
+        DOBIG4;
+        len -= 4;
+    }
+    buf4++;
+    buf = (const unsigned char FAR *)buf4;
+
+    if (len) do {
+        c = crc_table[4][(c >> 24) ^ *buf++] ^ (c << 8);
+    } while (--len);
+    c = ~c;
+    return (unsigned long)(REV(c));
+}
+
+#endif /* BYFOUR */
+
+#define GF2_DIM 32      /* dimension of GF(2) vectors (length of CRC) */
+
+/* ========================================================================= */
+local unsigned long gf2_matrix_times(mat, vec)
+    unsigned long *mat;
+    unsigned long vec;
+{
+    unsigned long sum;
+
+    sum = 0;
+    while (vec) {
+        if (vec & 1)
+            sum ^= *mat;
+        vec >>= 1;
+        mat++;
+    }
+    return sum;
+}
+
+/* ========================================================================= */
+local void gf2_matrix_square(square, mat)
+    unsigned long *square;
+    unsigned long *mat;
+{
+    int n;
+
+    for (n = 0; n < GF2_DIM; n++)
+        square[n] = gf2_matrix_times(mat, mat[n]);
+}
+
+/* ========================================================================= */
+uLong ZEXPORT crc32_combine(crc1, crc2, len2)
+    uLong crc1;
+    uLong crc2;
+    z_off_t len2;
+{
+    int n;
+    unsigned long row;
+    unsigned long even[GF2_DIM];    /* even-power-of-two zeros operator */
+    unsigned long odd[GF2_DIM];     /* odd-power-of-two zeros operator */
+
+    /* degenerate case */
+    if (len2 == 0)
+        return crc1;
+
+    /* put operator for one zero bit in odd */
+    odd[0] = 0xedb88320L;           /* CRC-32 polynomial */
+    row = 1;
+    for (n = 1; n < GF2_DIM; n++) {
+        odd[n] = row;
+        row <<= 1;
+    }
+
+    /* put operator for two zero bits in even */
+    gf2_matrix_square(even, odd);
+
+    /* put operator for four zero bits in odd */
+    gf2_matrix_square(odd, even);
+
+    /* apply len2 zeros to crc1 (first square will put the operator for one
+       zero byte, eight zero bits, in even) */
+    do {
+        /* apply zeros operator for this bit of len2 */
+        gf2_matrix_square(even, odd);
+        if (len2 & 1)
+            crc1 = gf2_matrix_times(even, crc1);
+        len2 >>= 1;
+
+        /* if no more bits set, then done */
+        if (len2 == 0)
+            break;
+
+        /* another iteration of the loop with odd and even swapped */
+        gf2_matrix_square(odd, even);
+        if (len2 & 1)
+            crc1 = gf2_matrix_times(odd, crc1);
+        len2 >>= 1;
+
+        /* if no more bits set, then done */
+    } while (len2 != 0);
+
+    /* return combined crc */
+    crc1 ^= crc2;
+    return crc1;
 }
