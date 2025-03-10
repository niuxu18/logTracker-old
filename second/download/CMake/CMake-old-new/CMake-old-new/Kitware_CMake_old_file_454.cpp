/*-
 * Copyright (c) 2011 Michihiro NAKAJIMA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "archive_platform.h"
__FBSDID("$FreeBSD$");

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_BZLIB_H
#include <bzlib.h>
#endif
#ifdef HAVE_LZMA_H
#include <lzma.h>
#endif
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

#include "archive.h"
#include "archive_entry.h"
#include "archive_entry_locale.h"
#include "archive_ppmd7_private.h"
#include "archive_private.h"
#include "archive_read_private.h"
#include "archive_endian.h"

#ifndef HAVE_ZLIB_H
#include "archive_crc32.h"
#endif

#define _7ZIP_SIGNATURE	"7z\xBC\xAF\x27\x1C"
#define SFX_MIN_ADDR	0x27000
#define SFX_MAX_ADDR	0x60000


/*
 * Codec ID
 */
#define _7Z_COPY	0
#define _7Z_LZMA	0x030101
#define _7Z_LZMA2	0x21
#define _7Z_DEFLATE	0x040108
#define _7Z_BZ2		0x040202
#define _7Z_PPMD	0x030401
#define _7Z_DELTA	0x03
#define _7Z_CRYPTO	0x06F10701
#define _7Z_X86		0x03030103
#define _7Z_X86_BCJ2	0x0303011B
#define _7Z_POWERPC	0x03030205
#define _7Z_IA64	0x03030401
#define _7Z_ARM		0x03030501
#define _7Z_ARMTHUMB	0x03030701
#define _7Z_SPARC	0x03030805

/*
 * 7-Zip header property IDs.
 */
#define kEnd			0x00
#define kHeader			0x01
#define kArchiveProperties	0x02
#define kAdditionalStreamsInfo	0x03
#define kMainStreamsInfo	0x04
#define kFilesInfo		0x05
#define kPackInfo		0x06
#define kUnPackInfo		0x07
#define kSubStreamsInfo		0x08
#define kSize			0x09
#define kCRC			0x0A
#define kFolder			0x0B
#define kCodersUnPackSize	0x0C
#define kNumUnPackStream	0x0D
#define kEmptyStream		0x0E
#define kEmptyFile		0x0F
#define kAnti			0x10
#define kName			0x11
#define kCTime			0x12
#define kATime			0x13
#define kMTime			0x14
#define kAttributes		0x15
#define kEncodedHeader		0x17

struct _7z_digests {
	unsigned char	*defineds;
	uint32_t	*digests;
};


struct _7z_folder {
	uint64_t		 numCoders;
	struct _7z_coder {
		unsigned long	 codec;
		uint64_t	 numInStreams;
		uint64_t	 numOutStreams;
		uint64_t	 propertiesSize;
		unsigned char	*properties;
	} *coders;
	uint64_t		 numBindPairs;
	struct {
		uint64_t	 inIndex;
		uint64_t	 outIndex;
	} *bindPairs;
	uint64_t		 numPackedStreams;
	uint64_t		*packedStreams;
	uint64_t		 numInStreams;
	uint64_t		 numOutStreams;
	uint64_t		*unPackSize;
	unsigned char		 digest_defined;
	uint32_t		 digest;
	uint64_t		 numUnpackStreams;
	uint32_t		 packIndex;
	/* Unoperated bytes. */
	uint64_t		 skipped_bytes;
};

struct _7z_coders_info {
	uint64_t		 numFolders;
	struct _7z_folder	*folders;
	uint64_t		 dataStreamIndex;
};

struct _7z_pack_info {
	uint64_t		 pos;
	uint64_t		 numPackStreams;
	uint64_t		*sizes;
	struct _7z_digests	 digest;
	/* Calculated from pos and numPackStreams. */
	uint64_t		*positions;
};

struct _7z_substream_info {
	size_t			 unpack_streams;
	uint64_t		*unpackSizes;
	unsigned char		*digestsDefined;
	uint32_t		*digests;
};

struct _7z_stream_info {
	struct _7z_pack_info	 pi;
	struct _7z_coders_info	 ci;
	struct _7z_substream_info ss;
};

struct _7z_header_info {
	uint64_t		 dataIndex;

	unsigned char		*emptyStreamBools;
	unsigned char		*emptyFileBools;
	unsigned char		*antiBools;
	unsigned char		*attrBools;
};

struct _7zip_entry {
	size_t			 name_len;
	unsigned char		*utf16name;
#if defined(_WIN32) && !defined(__CYGWIN__) && defined(_DEBUG)
	const wchar_t		*wname;
#endif
	uint32_t		 folderIndex;
	uint32_t		 ssIndex;
	unsigned		 flg;
#define MTIME_IS_SET	(1<<0)
#define ATIME_IS_SET	(1<<1)
#define CTIME_IS_SET	(1<<2)
#define CRC32_IS_SET	(1<<3)
#define HAS_STREAM	(1<<4)

	time_t			 mtime;
	time_t			 atime;
	time_t			 ctime;
	long			 mtime_ns;
	long			 atime_ns;
	long			 ctime_ns;
	uint32_t		 mode;
	uint32_t		 attr;
};

struct _7zip {
	/* Structural information about the archive. */
	struct _7z_stream_info	 si;

	/* Header offset to check that reading pointes of the file contens
	 * will not exceed the header. */
	uint64_t		 header_offset;
	/* Base offset of the archive file for a seek in case reading SFX. */
	uint64_t		 seek_base;

	/* List of entries */
	size_t			 entries_remaining;
	uint64_t		 numFiles;
	struct _7zip_entry	*entries;
	struct _7zip_entry	*entry;
	unsigned char		*entry_names;

	/* entry_bytes_remaining is the number of bytes we expect. */
	int64_t			 entry_offset;
	uint64_t		 entry_bytes_remaining;

	/* Running CRC32 of the decompressed data */
	unsigned long		 entry_crc32;

	/* Flags to mark progress of decompression. */
	char			 end_of_entry;

	/* Uncompressed buffer control.  */
	unsigned char 		*uncompressed_buffer;
	unsigned char 		*uncompressed_buffer_pointer;
	size_t 			 uncompressed_buffer_size;
	size_t			 uncompressed_buffer_bytes_remaining;

	/* Offset of the compressed data. */
	int64_t			 stream_offset;

	/*
	 * Decompressing control data.
	 */
	unsigned		 folder_index;
	uint64_t		 folder_outbytes_remaining;
	unsigned		 pack_stream_index;
	unsigned		 pack_stream_remaining;
	uint64_t		 pack_stream_inbytes_remaining;
	size_t			 pack_stream_bytes_unconsumed;

	/* The codec information of a folder. */
	unsigned long		 codec;
	unsigned long		 codec2;

	/*
	 * Decompressor controllers.
	 */
	/* Decording LZMA1 and LZMA2 data. */
#ifdef HAVE_LZMA_H
	lzma_stream		 lzstream;
	int			 lzstream_valid;
#endif
	/* Decording bzip2 data. */
#if defined(HAVE_BZLIB_H) && defined(BZ_CONFIG_ERROR)
	bz_stream		 bzstream;
	int			 bzstream_valid;
#endif
	/* Decording deflate data. */
#ifdef HAVE_ZLIB_H
	z_stream		 stream;
	int			 stream_valid;
#endif
	/* Decording PPMd data. */
	int			 ppmd7_stat;
	CPpmd7			 ppmd7_context;
	CPpmd7z_RangeDec	 range_dec;
	IByteIn			 bytein;
	struct {
		const unsigned char	*next_in;
		int64_t			 avail_in;
		int64_t			 total_in;
		unsigned char		*next_out;
		int64_t			 avail_out;
		int64_t			 total_out;
		int			 overconsumed;
	} ppstream;
	int			 ppmd7_valid;

	/* Decoding BCJ and BCJ2 data. */
	uint32_t		 bcj_state;
	size_t			 odd_bcj_size;
	unsigned char		 odd_bcj[4];

	/* Decoding BCJ2 data. */
	size_t			 main_stream_bytes_remaining;
	unsigned char		*sub_stream_buff[3];
	size_t			 sub_stream_size[3];
	size_t			 sub_stream_bytes_remaining[3];
	unsigned char		*tmp_stream_buff;
	size_t			 tmp_stream_buff_size;
	size_t			 tmp_stream_bytes_avail;
	size_t			 tmp_stream_bytes_remaining;
#ifdef _LZMA_PROB32
#define CProb uint32_t
#else
#define CProb uint16_t
#endif
	CProb			 bcj2_p[256 + 2];
	uint8_t			 bcj2_prevByte;
	uint32_t		 bcj2_range;
	uint32_t		 bcj2_code;
	uint64_t		 bcj2_outPos;

	/* Filename character-set convertion data. */
	struct archive_string_conv *sconv;

	char			 format_name[64];
};

static int	archive_read_format_7zip_bid(struct archive_read *, int);
static int	archive_read_format_7zip_cleanup(struct archive_read *);
static int	archive_read_format_7zip_read_data(struct archive_read *,
		    const void **, size_t *, int64_t *);
static int	archive_read_format_7zip_read_data_skip(struct archive_read *);
static int	archive_read_format_7zip_read_header(struct archive_read *,
		    struct archive_entry *);
static int	check_7zip_header_in_sfx(const char *);
static unsigned long decode_codec_id(const unsigned char *, size_t);
static ssize_t	decode_header_image(struct archive_read *, struct _7zip *,
		    struct _7z_stream_info *, const unsigned char *, uint64_t,
		    const void **);
static int	decompress(struct archive_read *, struct _7zip *,
		    void *, size_t *, const void *, size_t *);
static ssize_t	extract_pack_stream(struct archive_read *);
static void	fileTimeToUtc(uint64_t, time_t *, long *);
static uint64_t folder_uncompressed_size(struct _7z_folder *);
static void	free_CodersInfo(struct _7z_coders_info *);
static void	free_Digest(struct _7z_digests *);
static void	free_Folder(struct _7z_folder *);
static void	free_Header(struct _7z_header_info *);
static void	free_PackInfo(struct _7z_pack_info *);
static void	free_StreamsInfo(struct _7z_stream_info *);
static void	free_SubStreamsInfo(struct _7z_substream_info *);
static int	free_decompression(struct archive_read *, struct _7zip *);
static ssize_t	get_uncompressed_data(struct archive_read *, const void **,
		    size_t);
static int	init_decompression(struct archive_read *, struct _7zip *,
		    const struct _7z_coder *, const struct _7z_coder *);
static int	parse_7zip_uint64(const unsigned char *, size_t, uint64_t *);
static int	read_Bools(unsigned char *, size_t, const unsigned char *,
		    size_t);
static int	read_CodersInfo(struct _7z_coders_info *,
		    const unsigned char *, size_t);
static int	read_Digests(struct _7z_digests *, size_t,
		    const unsigned char *, size_t);
static int	read_Folder(struct _7z_folder *, const unsigned char *,
		    size_t);
static int	read_Header(struct _7zip *, struct _7z_header_info *,
		    const unsigned char *, size_t);
static int	read_PackInfo(struct _7z_pack_info *, const unsigned char *,
		    size_t);
static int	read_StreamsInfo(struct _7zip *, struct _7z_stream_info *,
		    const unsigned char *, size_t);
static int	read_SubStreamsInfo(struct _7z_substream_info *,
		    struct _7z_folder *, size_t, const unsigned char *,
		    size_t);
static int	read_Times(struct _7zip *, struct _7z_header_info *, int,
		    const unsigned char *, size_t);
static void	read_consume(struct archive_read *);
static ssize_t	read_stream(struct archive_read *, const void **, size_t);
static int64_t	skip_stream(struct archive_read *, size_t);
static int	skip_sfx(struct archive_read *, ssize_t);
static int	slurp_central_directory(struct archive_read *, struct _7zip *,
		    struct _7z_header_info *);
static int	setup_decode_folder(struct archive_read *, struct _7z_folder *,
		    int);
static size_t	x86_Convert(uint8_t *, size_t, uint32_t, uint32_t *);
ssize_t		Bcj2_Decode(struct _7zip *, uint8_t *, size_t);


int
archive_read_support_format_7zip(struct archive *_a)
{
	struct archive_read *a = (struct archive_read *)_a;
	struct _7zip *zip;
	int r;

	archive_check_magic(_a, ARCHIVE_READ_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_read_support_format_7zip");

	zip = calloc(1, sizeof(*zip));
	if (zip == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate 7zip data");
		return (ARCHIVE_FATAL);
	}

	r = __archive_read_register_format(a,
	    zip,
	    "7zip",
	    archive_read_format_7zip_bid,
	    NULL,
	    archive_read_format_7zip_read_header,
	    archive_read_format_7zip_read_data,
	    archive_read_format_7zip_read_data_skip,
	    archive_read_format_7zip_cleanup);

	if (r != ARCHIVE_OK)
		free(zip);
	return (ARCHIVE_OK);
}

static int
archive_read_format_7zip_bid(struct archive_read *a, int best_bid)
{
	const char *p;

	/* If someone has already bid more than 32, then avoid
	   trashing the look-ahead buffers with a seek. */
	if (best_bid > 32)
		return (-1);

	if ((p = __archive_read_ahead(a, 6, NULL)) == NULL)
		return (0);

	/* If first six bytes are the 7-Zip signature,
	 * return the bid right now. */
	if (memcmp(p, _7ZIP_SIGNATURE, 6) == 0)
		return (48);

	/*
	 * It may a 7-Zip SFX archive file. If first two bytes are
	 * 'M' and 'Z' available on Windows or first four bytes are
	 * "\x7F\x45LF" available on posix like system, seek the 7-Zip
	 * signature. Although we will perform a seek when reading
	 * a header, what we do not use __archive_read_seek() here is
	 * due to a bidding performance.
	 */
	if ((p[0] == 'M' && p[1] == 'Z') || memcmp(p, "\x7F\x45LF", 4) == 0) {
		ssize_t offset = SFX_MIN_ADDR;
		ssize_t window = 4096;
		ssize_t bytes_avail;
		while (offset + window <= (SFX_MAX_ADDR)) {
			const char *buff = __archive_read_ahead(a,
					offset + window, &bytes_avail);
			if (buff == NULL) {
				/* Remaining bytes are less than window. */
				window >>= 1;
				if (window < 0x40)
					return (0);
				continue;
			}
			p = buff + offset;
			while (p + 32 < buff + bytes_avail) {
				int step = check_7zip_header_in_sfx(p);
				if (step == 0)
					return (48);
				p += step;
			}
			offset = p - buff;
		}
	}
	return (0);
}

static int
check_7zip_header_in_sfx(const char *p)
{
	switch ((unsigned char)p[5]) {
	case 0x1C:
		if (memcmp(p, _7ZIP_SIGNATURE, 6) != 0)
			return (6); 
		/*
		 * Test the CRC because its extraction code has 7-Zip
		 * Magic Code, so we should do this in order not to
		 * make a mis-detection.
		 */
		if (crc32(0, (unsigned char *)p + 12, 20)
			!= archive_le32dec(p + 8))
			return (6); 
		/* Hit the header! */
		return (0);
	case 0x37: return (5); 
	case 0x7A: return (4); 
	case 0xBC: return (3); 
	case 0xAF: return (2); 
	case 0x27: return (1); 
	default: return (6); 
	}
}

static int
skip_sfx(struct archive_read *a, ssize_t bytes_avail)
{
	const void *h;
	const char *p, *q;
	size_t skip, offset;
	ssize_t bytes, window;

	/*
	 * If bytes_avail > SFX_MIN_ADDR we do not have to call
	 * __archive_read_seek() at this time since we have
	 * alredy had enough data.
	 */
	if (bytes_avail > SFX_MIN_ADDR)
		__archive_read_consume(a, SFX_MIN_ADDR);
	else if (__archive_read_seek(a, SFX_MIN_ADDR, SEEK_SET) < 0)
		return (ARCHIVE_FATAL);

	offset = 0;
	window = 1;
	while (offset + window <= SFX_MAX_ADDR - SFX_MIN_ADDR) {
		h = __archive_read_ahead(a, window, &bytes);
		if (h == NULL) {
			/* Remaining bytes are less than window. */
			window >>= 1;
			if (window < 0x40)
				goto fatal;
			continue;
		}
		if (bytes < 6) {
			/* This case might happen when window == 1. */
			window = 4096;
			continue;
		}
		p = (const char *)h;
		q = p + bytes;

		/*
		 * Scan ahead until we find something that looks
		 * like the 7-Zip header.
		 */
		while (p + 32 < q) {
			int step = check_7zip_header_in_sfx(p);
			if (step == 0) {
				struct _7zip *zip =
				    (struct _7zip *)a->format->data;
				skip = p - (const char *)h;
				__archive_read_consume(a, skip);
				zip->seek_base = SFX_MIN_ADDR + offset + skip;
				return (ARCHIVE_OK);
			}
			p += step;
		}
		skip = p - (const char *)h;
		__archive_read_consume(a, skip);
		offset += skip;
		if (window == 1)
			window = 4096;
	}
fatal:
	archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
	    "Couldn't find out 7-Zip header");
	return (ARCHIVE_FATAL);
}

static int
archive_read_format_7zip_read_header(struct archive_read *a,
	struct archive_entry *entry)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	struct _7zip_entry *zip_entry;
	int r, ret = ARCHIVE_OK;

	a->archive.archive_format = ARCHIVE_FORMAT_7ZIP;
	if (a->archive.archive_format_name == NULL)
		a->archive.archive_format_name = "7-Zip";

	if (zip->entries == NULL) {
		struct _7z_header_info header;

		memset(&header, 0, sizeof(header));
		r = slurp_central_directory(a, zip, &header);
		free_Header(&header);
		if (r != ARCHIVE_OK)
			return (r);
		zip->entries_remaining = zip->numFiles;
		zip->entry = zip->entries;
	} else {
		++zip->entry;
	}
	zip_entry = zip->entry;

	if (zip->entries_remaining <= 0)
		return ARCHIVE_EOF;
	--zip->entries_remaining;

	zip->entry_offset = 0;
	zip->end_of_entry = 0;
	zip->entry_crc32 = crc32(0, NULL, 0);

	/* Setup a string conversion for a filename. */
	if (zip->sconv == NULL) {
		zip->sconv = archive_string_conversion_from_charset(
		    &a->archive, "UTF-16LE", 1);
		if (zip->sconv == NULL)
			return (ARCHIVE_FATAL);
	}

	if (archive_entry_copy_pathname_l(entry,
	    (const char *)zip_entry->utf16name,
	    zip_entry->name_len, zip->sconv) != 0) {
		if (errno == ENOMEM) {
			archive_set_error(&a->archive, ENOMEM,
			    "Can't allocate memory for Pathname");
			return (ARCHIVE_FATAL);
		}
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_FILE_FORMAT,
		    "Pathname cannot be converted "
		    "from %s to current locale.",
		    archive_string_conversion_charset_name(zip->sconv));
		ret = ARCHIVE_WARN;
	}

	/* Populate some additional entry fields: */
	archive_entry_set_mode(entry, zip_entry->mode);
	if (zip_entry->flg & MTIME_IS_SET)
		archive_entry_set_mtime(entry, zip_entry->mtime,
			zip_entry->mtime_ns);
	if (zip_entry->flg & CTIME_IS_SET)
		archive_entry_set_ctime(entry, zip_entry->ctime,
		    zip_entry->ctime_ns);
	if (zip_entry->flg & ATIME_IS_SET)
		archive_entry_set_atime(entry, zip_entry->atime,
		    zip_entry->atime_ns);
	if (zip_entry->ssIndex != -1) {
		zip->entry_bytes_remaining =
		    zip->si.ss.unpackSizes[zip_entry->ssIndex];
		archive_entry_set_size(entry, zip->entry_bytes_remaining);
	} else {
		zip->entry_bytes_remaining = 0;
		archive_entry_set_size(entry, 0);
	}

	/* If there's no body, force read_data() to return EOF immediately. */
	if (zip->entry_bytes_remaining < 1)
		zip->end_of_entry = 1;

	if ((zip_entry->mode & AE_IFMT) == AE_IFLNK) {
		unsigned char *symname = NULL;
		size_t symsize = 0;
		int r;

		/*
		 * Symbolic-name is recorded as its contents. We have to read the
		 * contents at this time.
		 */
		while (zip->entry_bytes_remaining > 0) {
			const void *buff;
			size_t size;
			int64_t offset;

			r = archive_read_format_7zip_read_data(a, &buff, &size,
				&offset);
			if (r < ARCHIVE_WARN)
				return (r);
			symname = realloc(symname, symsize + size + 1);
			if (symname == NULL) {
				archive_set_error(&a->archive, ENOMEM,
				    "Can't allocate memory for Symname");
				return (ARCHIVE_FATAL);
			}
			memcpy(symname+symsize, buff, size);
			symsize += size;
		}
		if (symsize == 0) {
			/* If there is no synname, handle it as a regular file. */
			zip_entry->mode &= ~AE_IFMT;
			zip_entry->mode |= AE_IFREG;
			archive_entry_set_mode(entry, zip_entry->mode);
		} else {
			symname[symsize] = '\0';
			archive_entry_copy_symlink(entry, (const char *)symname);
			free(symname);
		}
		archive_entry_set_size(entry, 0);
	}

	/* Set up a more descriptive format name. */
	sprintf(zip->format_name, "7-Zip");
	a->archive.archive_format_name = zip->format_name;

	return (ret);
}

static int
archive_read_format_7zip_read_data(struct archive_read *a,
    const void **buff, size_t *size, int64_t *offset)
{
	struct _7zip *zip;
	ssize_t bytes;
	int ret = ARCHIVE_OK;

	zip = (struct _7zip *)(a->format->data);

	if (zip->pack_stream_bytes_unconsumed)
		read_consume(a);

	/*
	 * If we hit end-of-entry last time, clean up and return
	 * ARCHIVE_EOF this time.
	 */
	if (zip->end_of_entry) {
		*offset = zip->entry_offset;
		*size = 0;
		*buff = NULL;
		return (ARCHIVE_EOF);
	}

	bytes = read_stream(a, buff, zip->entry_bytes_remaining);
	if (bytes < 0)
		return ((int)bytes);
	if (bytes == 0) {
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated 7-Zip file body");
		return (ARCHIVE_FATAL);
	}
	zip->entry_bytes_remaining -= bytes;
	if (zip->entry_bytes_remaining == 0)
		zip->end_of_entry = 1;

	/* Update checksum */
	if ((zip->entry->flg & CRC32_IS_SET) && bytes)
		zip->entry_crc32 = crc32(zip->entry_crc32, *buff, bytes);

	/* If we hit the end, swallow any end-of-data marker. */
	if (zip->end_of_entry) {
		/* Check computed CRC against file contents. */
		if ((zip->entry->flg & CRC32_IS_SET) &&
			zip->si.ss.digests[zip->entry->ssIndex] !=
		    zip->entry_crc32) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "7-Zip bad CRC: 0x%lx should be 0x%lx",
			    (unsigned long)zip->entry_crc32,
			    (unsigned long)zip->si.ss.digests[
			    		zip->entry->ssIndex]);
			ret = ARCHIVE_WARN;
		}
	}

	*size = bytes;
	*offset = zip->entry_offset;
	zip->entry_offset += bytes;

	return (ret);
}

static int
archive_read_format_7zip_read_data_skip(struct archive_read *a)
{
	struct _7zip *zip;
	int64_t bytes_skipped;

	zip = (struct _7zip *)(a->format->data);

	if (zip->pack_stream_bytes_unconsumed)
		read_consume(a);

	/* If we've already read to end of data, we're done. */
	if (zip->end_of_entry)
		return (ARCHIVE_OK);

	/*
	 * If the length is at the beginning, we can skip the
	 * compressed data much more quickly.
	 */
	bytes_skipped = skip_stream(a, zip->entry_bytes_remaining);
	if (bytes_skipped < 0)
		return (ARCHIVE_FATAL);
	zip->entry_bytes_remaining = 0;

	/* This entry is finished and done. */
	zip->end_of_entry = 1;
	return (ARCHIVE_OK);
}

static int
archive_read_format_7zip_cleanup(struct archive_read *a)
{
	struct _7zip *zip;

	zip = (struct _7zip *)(a->format->data);
	free_StreamsInfo(&(zip->si));
	free(zip->entries);
	free(zip->entry_names);
	free_decompression(a, zip);
	free(zip->uncompressed_buffer);
	free(zip->sub_stream_buff[0]);
	free(zip->sub_stream_buff[1]);
	free(zip->sub_stream_buff[2]);
	free(zip->tmp_stream_buff);
	free(zip);
	(a->format->data) = NULL;
	return (ARCHIVE_OK);
}

static void
read_consume(struct archive_read *a)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;

	if (zip->pack_stream_bytes_unconsumed) {
		__archive_read_consume(a, zip->pack_stream_bytes_unconsumed);
		zip->stream_offset += zip->pack_stream_bytes_unconsumed;
		zip->pack_stream_bytes_unconsumed = 0;
	}
}

#ifdef HAVE_LZMA_H

/*
 * Set an error code and choose an error message for liblzma.
 */
static void
set_error(struct archive_read *a, int ret)
{

	switch (ret) {
	case LZMA_STREAM_END: /* Found end of stream. */
	case LZMA_OK: /* Decompressor made some progress. */
		break;
	case LZMA_MEM_ERROR:
		archive_set_error(&a->archive, ENOMEM,
		    "Lzma library error: Cannot allocate memory");
		break;
	case LZMA_MEMLIMIT_ERROR:
		archive_set_error(&a->archive, ENOMEM,
		    "Lzma library error: Out of memory");
		break;
	case LZMA_FORMAT_ERROR:
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_MISC,
		    "Lzma library error: format not recognized");
		break;
	case LZMA_OPTIONS_ERROR:
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_MISC,
		    "Lzma library error: Invalid options");
		break;
	case LZMA_DATA_ERROR:
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_MISC,
		    "Lzma library error: Corrupted input data");
		break;
	case LZMA_BUF_ERROR:
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_MISC,
		    "Lzma library error:  No progress is possible");
		break;
	default:
		/* Return an error. */
		archive_set_error(&a->archive,
		    ARCHIVE_ERRNO_MISC,
		    "Lzma decompression failed:  Unknown error");
		break;
	}
}

#endif

static unsigned long
decode_codec_id(const unsigned char *codecId, size_t id_size)
{
	unsigned i;
	unsigned long id = 0;

	for (i = 0; i < id_size; i++) {
		id <<= 8;
		id += codecId[i];
	}
	return (id);
}

static void *
ppmd_alloc(void *p, size_t size)
{
	(void)p;
	return malloc(size);
}
static void
ppmd_free(void *p, void *address)
{
	(void)p;
	free(address);
}
static Byte
ppmd_read(void *p)
{
	struct archive_read *a = ((IByteIn*)p)->a;
	struct _7zip *zip = (struct _7zip *)(a->format->data);
	Byte b;

	if (zip->ppstream.avail_in == 0) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated RAR file data");
		zip->ppstream.overconsumed = 1;
		return (0);
	}
	b = *zip->ppstream.next_in++;
	zip->ppstream.avail_in--;
	zip->ppstream.total_in++;
	return (b);
}

static ISzAlloc g_szalloc = { ppmd_alloc, ppmd_free };

static int
init_decompression(struct archive_read *a, struct _7zip *zip,
    const struct _7z_coder *coder1, const struct _7z_coder *coder2)
{
	int r;

	zip->codec = coder1->codec;
	zip->codec2 = -1;

	switch (zip->codec) {
	case _7Z_COPY:
	case _7Z_BZ2:
	case _7Z_DEFLATE:
	case _7Z_PPMD:
		if (coder2 != NULL) {
			if (coder2->codec != _7Z_X86 &&
			    coder2->codec != _7Z_X86_BCJ2) {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "Unsupported filter %lx for %lx",
				    coder2->codec, coder1->codec);
				return (ARCHIVE_FAILED);
			}
			zip->codec2 = coder2->codec;
			zip->bcj_state = 0;
		}
		break;
	default:
		break;
	}

	switch (zip->codec) {
	case _7Z_COPY:
		break;

	case _7Z_LZMA: case _7Z_LZMA2:
#ifdef HAVE_LZMA_H
#if LZMA_VERSION_MAJOR >= 5
/* Effectively disable the limiter. */
#define LZMA_MEMLIMIT   UINT64_MAX
#else
/* NOTE: This needs to check memory size which running system has. */
#define LZMA_MEMLIMIT   (1U << 30)
#endif
	{
		lzma_options_delta delta_opt;
		lzma_filter filters[LZMA_FILTERS_MAX];
#if LZMA_VERSION < 50000030
		lzma_filter *ff;
#endif
		int fi = 0;

		if (zip->lzstream_valid) {
			lzma_end(&(zip->lzstream));
			zip->lzstream_valid = 0;
		}

		/*
		 * NOTE: liblzma incompletely handle the BCJ+LZMA compressed
		 * data made by 7-Zip because 7-Zip does not add End-Of-
		 * Payload Marker(EOPM) at the end of LZMA compressed data,
		 * and so liblzma cannot know the end of the compressed data
		 * without EOPM. So consequently liblzma will not return last
		 * three or four bytes of uncompressed data because
		 * LZMA_FILTER_X86 filter does not handle input data if its
		 * data size is less than five bytes. If liblzma detect EOPM
		 * or know the uncompressed data size, liblzma will flush out
		 * the remaining that three or four bytes of uncompressed
		 * data. That is why we have to use our converting program
		 * for BCJ+LZMA. If we were able to tell the uncompressed
		 * size to liblzma when using lzma_raw_decoder() liblzma
		 * could correctly deal with BCJ+LZMA. But unfortunately
		 * there is no way to do that. 
		 * Discussion about this can be found at XZ Utils forum.
		 */
		if (coder2 != NULL) {
			zip->codec2 = coder2->codec;

			filters[fi].options = NULL;
			switch (zip->codec2) {
			case _7Z_X86:
				if (zip->codec == _7Z_LZMA2) {
					filters[fi].id = LZMA_FILTER_X86;
					fi++;
				} else
					/* Use our filter. */
					zip->bcj_state = 0;
				break;
			case _7Z_X86_BCJ2:
				/* Use our filter. */
				zip->bcj_state = 0;
				break;
			case _7Z_DELTA:
				filters[fi].id = LZMA_FILTER_DELTA;
				memset(&delta_opt, 0, sizeof(delta_opt));
				delta_opt.type = LZMA_DELTA_TYPE_BYTE;
				delta_opt.dist = 1;
				filters[fi].options = &delta_opt;
				fi++;
				break;
			/* Following filters have not been tested yet. */
			case _7Z_POWERPC:
				filters[fi].id = LZMA_FILTER_POWERPC;
				fi++;
				break;
			case _7Z_IA64:
				filters[fi].id = LZMA_FILTER_IA64;
				fi++;
				break;
			case _7Z_ARM:
				filters[fi].id = LZMA_FILTER_ARM;
				fi++;
				break;
			case _7Z_ARMTHUMB:
				filters[fi].id = LZMA_FILTER_ARMTHUMB;
				fi++;
				break;
			case _7Z_SPARC:
				filters[fi].id = LZMA_FILTER_SPARC;
				fi++;
				break;
			default:
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "Unexpected codec ID: %lX", zip->codec2);
				return (ARCHIVE_FAILED);
			}
		}

		if (zip->codec == _7Z_LZMA2)
			filters[fi].id = LZMA_FILTER_LZMA2;
		else
			filters[fi].id = LZMA_FILTER_LZMA1;
		filters[fi].options = NULL;
#if LZMA_VERSION < 50000030
		ff = &filters[fi];
#endif
		r = lzma_properties_decode(&filters[fi], NULL,
		    coder1->properties, coder1->propertiesSize);
		if (r != LZMA_OK) {
			set_error(a, r);
			return (ARCHIVE_FAILED);
		}
		fi++;

		filters[fi].id = LZMA_VLI_UNKNOWN;
		filters[fi].options = NULL;
		r = lzma_raw_decoder(&(zip->lzstream), filters);
#if LZMA_VERSION < 50000030
		free(ff->options);
#endif
		if (r != LZMA_OK) {
			set_error(a, r);
			return (ARCHIVE_FAILED);
		}
		zip->lzstream_valid = 1;
		zip->lzstream.total_in = 0;
		zip->lzstream.total_out = 0;
		break;
	}
#else
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "LZMA codec is unsupported");
		return (ARCHIVE_FAILED);
#endif
	case _7Z_BZ2:
#if defined(HAVE_BZLIB_H) && defined(BZ_CONFIG_ERROR)
		if (zip->bzstream_valid) {
			BZ2_bzDecompressEnd(&(zip->bzstream));
			zip->bzstream_valid = 0;
		}
		r = BZ2_bzDecompressInit(&(zip->bzstream), 0, 0);
		if (r == BZ_MEM_ERROR)
			r = BZ2_bzDecompressInit(&(zip->bzstream), 0, 1);
		if (r != BZ_OK) {
			int err = ARCHIVE_ERRNO_MISC;
			const char *detail = NULL;
			switch (r) {
			case BZ_PARAM_ERROR:
				detail = "invalid setup parameter";
				break;
			case BZ_MEM_ERROR:
				err = ENOMEM;
				detail = "out of memory";
				break;
			case BZ_CONFIG_ERROR:
				detail = "mis-compiled library";
				break;
			}
			archive_set_error(&a->archive, err,
			    "Internal error initializing decompressor: %s",
			    detail == NULL ? "??" : detail);
			zip->bzstream_valid = 0;
			return (ARCHIVE_FAILED);
		}
		zip->bzstream_valid = 1;
		zip->bzstream.total_in_lo32 = 0;
		zip->bzstream.total_in_hi32 = 0;
		zip->bzstream.total_out_lo32 = 0;
		zip->bzstream.total_out_hi32 = 0;
		break;
#else
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "BZ2 codec is unsupported");
		return (ARCHIVE_FAILED);
#endif
	case _7Z_DEFLATE:
#ifdef HAVE_ZLIB_H
		if (zip->stream_valid)
			r = inflateReset(&(zip->stream));
		else
			r = inflateInit2(&(zip->stream),
			    -15 /* Don't check for zlib header */);
		if (r != Z_OK) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "Couldn't initialize zlib stream.");
			return (ARCHIVE_FAILED);
		}
		zip->stream_valid = 1;
		zip->stream.total_in = 0;
		zip->stream.total_out = 0;
		break;
#else
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "DEFLATE codec is unsupported");
		return (ARCHIVE_FAILED);
#endif
	case _7Z_PPMD:
	{
		unsigned order;
		uint32_t msize;

		if (zip->ppmd7_valid) {
			__archive_ppmd7_functions.Ppmd7_Free(
			    &zip->ppmd7_context, &g_szalloc);
			zip->ppmd7_valid = 0;
		}

		if (coder1->propertiesSize < 5) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "Malformed PPMd parameter");
			return (ARCHIVE_FAILED);
		}
		order = coder1->properties[0];
		msize = archive_le32dec(&(coder1->properties[1]));
		if (order < PPMD7_MIN_ORDER || order > PPMD7_MAX_ORDER ||
		    msize < PPMD7_MIN_MEM_SIZE || msize > PPMD7_MAX_MEM_SIZE) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "Malformed PPMd parameter");
			return (ARCHIVE_FAILED);
		}
		__archive_ppmd7_functions.Ppmd7_Construct(&zip->ppmd7_context);
		r = __archive_ppmd7_functions.Ppmd7_Alloc(
			&zip->ppmd7_context, msize, &g_szalloc);
		if (r == 0) {
			archive_set_error(&a->archive, ENOMEM,
			    "Coludn't allocate memory for PPMd");
			return (ARCHIVE_FATAL);
		}
		__archive_ppmd7_functions.Ppmd7_Init(
			&zip->ppmd7_context, order);
		__archive_ppmd7_functions.Ppmd7z_RangeDec_CreateVTable(
			&zip->range_dec);
		zip->ppmd7_valid = 1;
		zip->ppmd7_stat = 0;
		zip->ppstream.overconsumed = 0;
		zip->ppstream.total_in = 0;
		zip->ppstream.total_out = 0;
		break;
	}
	case _7Z_X86:
	case _7Z_X86_BCJ2:
	case _7Z_POWERPC:
	case _7Z_IA64:
	case _7Z_ARM:
	case _7Z_ARMTHUMB:
	case _7Z_SPARC:
	case _7Z_DELTA:
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Unexpected codec ID: %lX", zip->codec);
		return (ARCHIVE_FAILED);
	default:
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Unknown codec ID: %lX", zip->codec);
		return (ARCHIVE_FAILED);
	}

	return (ARCHIVE_OK);
}

static int
decompress(struct archive_read *a, struct _7zip *zip,
    void *buff, size_t *outbytes, const void *b, size_t *used)
{
	const uint8_t *t_next_in;
	uint8_t *t_next_out;
	size_t o_avail_in, o_avail_out;
	size_t t_avail_in, t_avail_out;
	uint8_t *bcj2_next_out;
	size_t bcj2_avail_out;
	int r, ret = ARCHIVE_OK;

	t_avail_in = o_avail_in = *used;
	t_avail_out = o_avail_out = *outbytes;
	t_next_in = b;
	t_next_out = buff;

	if (zip->codec != _7Z_LZMA2 && zip->codec2 == _7Z_X86) {
		int i;
		for (i = 0; zip->odd_bcj_size > 0 && t_avail_out; i++) {
			*t_next_out++ = zip->odd_bcj[i];
			t_avail_out--;
			zip->odd_bcj_size--;
		}
		if (o_avail_in == 0 || t_avail_out == 0) {
			*used = o_avail_in - t_avail_in;
			*outbytes = o_avail_out - t_avail_out;
			if (o_avail_in == 0)
				ret = ARCHIVE_EOF;
			return (ret);
		}
	}

	bcj2_next_out = t_next_out;
	bcj2_avail_out = t_avail_out;
	if (zip->codec2 == _7Z_X86_BCJ2) {
		/*
		 * Decord a remaining decompressed main stream for BCJ2.
		 */
		if (zip->tmp_stream_bytes_remaining > 0) {
			ssize_t bytes;
			size_t remaining = zip->tmp_stream_bytes_remaining;
			bytes = Bcj2_Decode(zip, t_next_out, t_avail_out);
			if (bytes < 0) {
				archive_set_error(&(a->archive),
				    ARCHIVE_ERRNO_MISC,
				    "BCJ2 conversion Failed");
				return (ARCHIVE_FAILED);
			}
			zip->main_stream_bytes_remaining -=
			    remaining - zip->tmp_stream_bytes_remaining;
			t_avail_out -= bytes;
			if (o_avail_in == 0 || t_avail_out == 0) {
				*used = 0;
				*outbytes = o_avail_out - t_avail_out;
				if (o_avail_in == 0)
					ret = ARCHIVE_EOF;
				return (ret);
			}
			t_next_out += bytes;
			bcj2_next_out = t_next_out;
			bcj2_avail_out = t_avail_out;
		}
		t_next_out = zip->tmp_stream_buff;
		t_avail_out = zip->tmp_stream_buff_size;
	}

	switch (zip->codec) {
	case _7Z_COPY:
	{
		size_t bytes =
		    (t_avail_in > t_avail_out)?t_avail_out:t_avail_in;

		memcpy(t_next_out, t_next_in, bytes);
		t_avail_in -= bytes;
		t_avail_out -= bytes;
		if (o_avail_in == 0)
			ret = ARCHIVE_EOF;
		break;
	}
#ifdef HAVE_LZMA_H
	case _7Z_LZMA: case _7Z_LZMA2:
		zip->lzstream.next_in = t_next_in;
		zip->lzstream.avail_in = t_avail_in;
		zip->lzstream.next_out = t_next_out;
		zip->lzstream.avail_out = t_avail_out;

		r = lzma_code(&(zip->lzstream), LZMA_RUN);
		switch (r) {
		case LZMA_STREAM_END: /* Found end of stream. */
			lzma_end(&(zip->lzstream));
			zip->lzstream_valid = 0;
			ret = ARCHIVE_EOF;
			break;
		case LZMA_OK: /* Decompressor made some progress. */
			break;
		default:
			archive_set_error(&(a->archive),
			    ARCHIVE_ERRNO_MISC,
				"Decompression failed(%d)",
			    r);
			return (ARCHIVE_FAILED);
		}
		t_avail_in = zip->lzstream.avail_in;
		t_avail_out = zip->lzstream.avail_out;
		break;
#endif
#if defined(HAVE_BZLIB_H) && defined(BZ_CONFIG_ERROR)
	case _7Z_BZ2:
		zip->bzstream.next_in = (char *)(uintptr_t)t_next_in;
		zip->bzstream.avail_in = t_avail_in;
		zip->bzstream.next_out = (char *)(uintptr_t)t_next_out;
		zip->bzstream.avail_out = t_avail_out;
		r = BZ2_bzDecompress(&(zip->bzstream));
		switch (r) {
		case BZ_STREAM_END: /* Found end of stream. */
			switch (BZ2_bzDecompressEnd(&(zip->bzstream))) {
			case BZ_OK:
				break;
			default:
				archive_set_error(&(a->archive),
				    ARCHIVE_ERRNO_MISC,
				    "Failed to clean up decompressor");
				return (ARCHIVE_FAILED);
			}
			zip->bzstream_valid = 0;
			ret = ARCHIVE_EOF;
			break;
		case BZ_OK: /* Decompressor made some progress. */
			break;
		default:
			archive_set_error(&(a->archive),
			    ARCHIVE_ERRNO_MISC,
			    "bzip decompression failed");
			return (ARCHIVE_FAILED);
		}
		t_avail_in = zip->bzstream.avail_in;
		t_avail_out = zip->bzstream.avail_out;
		break;
#endif
#ifdef HAVE_ZLIB_H
	case _7Z_DEFLATE:
		zip->stream.next_in = (Bytef *)(uintptr_t)t_next_in;
		zip->stream.avail_in = t_avail_in;
		zip->stream.next_out = t_next_out;
		zip->stream.avail_out = t_avail_out;
		r = inflate(&(zip->stream), 0);
		switch (r) {
		case Z_STREAM_END: /* Found end of stream. */
			ret = ARCHIVE_EOF;
			break;
		case Z_OK: /* Decompressor made some progress.*/
			break;
		default:
			archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
			    "File decompression failed (%d)", r);
			return (ARCHIVE_FAILED);
		}
		t_avail_in = zip->stream.avail_in;
		t_avail_out = zip->stream.avail_out;
		break;
#endif
	case _7Z_PPMD:
	{
		uint64_t flush_bytes;

		if (!zip->ppmd7_valid || zip->ppmd7_stat < 0 ||
		    t_avail_in < 0 || t_avail_out <= 0) {
			archive_set_error(&(a->archive),
			    ARCHIVE_ERRNO_MISC,
			    "Decompression internal error");
			return (ARCHIVE_FAILED);
		}
		zip->ppstream.next_in = t_next_in;
		zip->ppstream.avail_in = t_avail_in;
		zip->ppstream.next_out = t_next_out;
		zip->ppstream.avail_out = t_avail_out;
		if (zip->ppmd7_stat == 0) {
			zip->bytein.a = a;
			zip->bytein.Read = &ppmd_read;
			zip->range_dec.Stream = &zip->bytein;
			r = __archive_ppmd7_functions.Ppmd7z_RangeDec_Init(
				&(zip->range_dec));
			if (r == 0) {
				zip->ppmd7_stat = -1;
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "Failed to initialize PPMd range decorder");
				return (ARCHIVE_FAILED);
			}
			if (zip->ppstream.overconsumed) {
				zip->ppmd7_stat = -1;
				return (ARCHIVE_FAILED);
			}
			zip->ppmd7_stat = 1;
		}

		if (t_avail_in == 0)
			/* XXX Flush out remaining decoded data XXX */
			flush_bytes = zip->folder_outbytes_remaining;
		else
			flush_bytes = 0;

		do {
			int sym;
			
			sym = __archive_ppmd7_functions.Ppmd7_DecodeSymbol(
				&(zip->ppmd7_context), &(zip->range_dec.p));
			if (sym < 0) {
				zip->ppmd7_stat = -1;
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_FILE_FORMAT,
				    "Failed to decode PPMd");
				return (ARCHIVE_FAILED);
			}
			if (zip->ppstream.overconsumed) {
				zip->ppmd7_stat = -1;
				return (ARCHIVE_FAILED);
			}
			*zip->ppstream.next_out++ = (unsigned char)sym;
			zip->ppstream.avail_out--;
			zip->ppstream.total_out++;
			if (flush_bytes)
				flush_bytes--;
		} while (zip->ppstream.avail_out &&
			(zip->ppstream.avail_in || flush_bytes));

		t_avail_in = zip->ppstream.avail_in;
		t_avail_out = zip->ppstream.avail_out;
		break;
	}
	default:
		archive_set_error(&(a->archive), ARCHIVE_ERRNO_MISC,
		    "Decompression internal error");
		return (ARCHIVE_FAILED);
	}
	if (ret != ARCHIVE_OK && ret != ARCHIVE_EOF)
		return (ret);

	*used = o_avail_in - t_avail_in;
	*outbytes = o_avail_out - t_avail_out;

	/*
	 * Decord BCJ.
	 */
	if (zip->codec != _7Z_LZMA2 && zip->codec2 == _7Z_X86) {
		size_t l = x86_Convert(buff, *outbytes, 0, &(zip->bcj_state));
		zip->odd_bcj_size = *outbytes - l;
		if (zip->odd_bcj_size > 0 && zip->odd_bcj_size <= 4 &&
		    o_avail_in && ret != ARCHIVE_EOF) {
			memcpy(zip->odd_bcj, ((unsigned char *)buff) + l,
			    zip->odd_bcj_size);
			*outbytes = l;
		} else
			zip->odd_bcj_size = 0;
	}

	/*
	 * Decord BCJ2 with a decompressed main stream.
	 */
	if (zip->codec2 == _7Z_X86_BCJ2) {
		ssize_t bytes;

		zip->tmp_stream_bytes_avail =
		    zip->tmp_stream_buff_size - t_avail_out;
		if (zip->tmp_stream_bytes_avail >
		      zip->main_stream_bytes_remaining)
			zip->tmp_stream_bytes_avail =
			    zip->main_stream_bytes_remaining;
		zip->tmp_stream_bytes_remaining = zip->tmp_stream_bytes_avail;
		bytes = Bcj2_Decode(zip, bcj2_next_out, bcj2_avail_out);
		if (bytes < 0) {
			archive_set_error(&(a->archive),
			    ARCHIVE_ERRNO_MISC, "BCJ2 conversion Failed");
			return (ARCHIVE_FAILED);
		}
		zip->main_stream_bytes_remaining -=
		    zip->tmp_stream_bytes_avail
		      - zip->tmp_stream_bytes_remaining;
		bcj2_avail_out -= bytes;
		*outbytes = o_avail_out - bcj2_avail_out;
	}

	return (ret);
}

static int
free_decompression(struct archive_read *a, struct _7zip *zip)
{
	int r = ARCHIVE_OK;

#ifdef HAVE_LZMA_H
	if (zip->lzstream_valid)
		lzma_end(&(zip->lzstream));
#endif
#if defined(HAVE_BZLIB_H) && defined(BZ_CONFIG_ERROR)
	if (zip->bzstream_valid) {
		if (BZ2_bzDecompressEnd(&(zip->bzstream)) != BZ_OK) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_MISC,
			    "Failed to clean up bzip2 decompressor");
			r = ARCHIVE_FATAL;
		}
		zip->bzstream_valid = 0;
	}
#endif
#ifdef HAVE_ZLIB_H
	if (zip->stream_valid) {
		if (inflateEnd(&(zip->stream)) != Z_OK) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_MISC,
			    "Failed to clean up zlib decompressor");
			r = ARCHIVE_FATAL;
		}
		zip->stream_valid = 0;
	}
#endif
	if (zip->ppmd7_valid) {
		__archive_ppmd7_functions.Ppmd7_Free(
			&zip->ppmd7_context, &g_szalloc);
		zip->ppmd7_valid = 0;
	}
	return (r);
}

static int
parse_7zip_uint64(const unsigned char *p, size_t len, uint64_t *val)
{
	const unsigned char *_p = p;
	unsigned char avail, mask;
	int i;

	if (len-- == 0)
		return (-1);
	avail = *p++;
	mask = 0x80;
	*val = 0;
	for (i = 0; i < 8; i++) {
		if (avail & mask) {
			if (len-- == 0)
				return (-1);
			*val |= ((uint64_t)*p++) << (8 * i);
			mask >>= 1;
			continue;
		}
		*val += (avail & (mask -1)) << (8 * i);
		break;
	}
	return (p - _p);
}

static int
read_Bools(unsigned char *data, size_t num, const unsigned char *p, size_t len)
{
	const unsigned char *_p = p;
	unsigned i, mask = 0, avail;

	for (i = 0; i < num; i++) {
		if (mask == 0) {
			if (len == 0)
				return (-1);
			avail = *p++;
			len--;
			mask = 0x80;
		}
		data[i] = (avail & mask)?1:0;
		mask >>= 1;
	}
	return (p - _p);
}

static void
free_Digest(struct _7z_digests *d)
{
	free(d->defineds);
	free(d->digests);
}

static int
read_Digests(struct _7z_digests *d, size_t num, const unsigned char *p,
    size_t len)
{
	const unsigned char *_p = p;
	unsigned i;

	memset(d, 0, sizeof(*d));

	if (len == 0)
		return (-1);

	d->defineds = malloc(num);
	if (d->defineds == NULL)
		return (-1);
	/*
	 * Read Bools.
	 */
	len--;
	if (*p++ == 0) {
		int r = read_Bools(d->defineds, num, p, len);
		if (r < 0)
			return (-1);
		p += r;
		len -= r;
	} else
		/* All are defined */
		memset(d->defineds, 1, num);

	d->digests = calloc(num, sizeof(*d->digests));
	if (d->digests == NULL)
		return (-1);
	if (len < 4 * num)
		return (-1);
	for (i = 0; i < num; i++) {
		if (d->defineds[i]) {
			d->digests[i] = archive_le32dec(p);
			p += 4;
			len -= 4;
		}
	}

	return (p - _p);
}

static void
free_PackInfo(struct _7z_pack_info *pi)
{
	free(pi->sizes);
	free(pi->positions);
	free_Digest(&(pi->digest));
}

static int
read_PackInfo(struct _7z_pack_info *pi, const unsigned char *p, size_t len)
{
	const unsigned char *_p = p;
	unsigned i;
	int r;

	memset(pi, 0, sizeof(*pi));

	if (len < 3 || *p++ != kPackInfo)
		return (-1);
	--len;

	/*
	 * Read PackPos.
	 */
	r = parse_7zip_uint64(p, len, &(pi->pos));
	if (r < 0)
		return (r);
	p += r;
	len -= r;

	/*
	 * Read NumPackStreams.
	 */
	r = parse_7zip_uint64(p, len, &(pi->numPackStreams));
	if (r < 0 || pi->numPackStreams == 0)
		return (r);
	p += r;
	len -= r;

	/*
	 * Read PackSizes[num]
	 */
	if (len >= 1 && *p == kEnd)
		/* PackSizes[num] are not present. */
		return (p - _p + 1);
	if (len < 1 + pi->numPackStreams || *p++ != kSize)
		return (-1);
	--len;
	pi->sizes = calloc(pi->numPackStreams, sizeof(uint64_t));
	pi->positions = calloc(pi->numPackStreams, sizeof(uint64_t));
	if (pi->sizes == NULL || pi->positions == NULL)
		return (-1);

	for (i = 0; i < pi->numPackStreams; i++) {
		r = parse_7zip_uint64(p, len, &(pi->sizes[i]));
		if (r < 0)
			return (-1);
		p += r;
		len -= r;
	}

	/*
	 * Read PackStreamDigests[num]
	 */
	if (len >= 1 && *p == kEnd) {
		/* PackStreamDigests[num] are not present. */
		pi->digest.defineds =
		    calloc(pi->numPackStreams, sizeof(*pi->digest.defineds));
		pi->digest.digests =
		    calloc(pi->numPackStreams, sizeof(*pi->digest.digests));
		if (pi->digest.defineds == NULL || pi->digest.digests == NULL)
			return (-1);
		return (p - _p + 1);
	}

	if (len < 1 + pi->numPackStreams || *p++ != kSize)
		return (-1);
	--len;

	r = read_Digests(&(pi->digest), pi->numPackStreams, p, len);
	if (r < 0)
		return (-1);
	p += r;
	len -= r;

	/*
	 *  Must be marked by kEnd.
	 */
	if (len == 0 || *p++ != kEnd)
		return (-1);
	return (p - _p);
}

static void
free_Folder(struct _7z_folder *f)
{
	unsigned i;

	if (f->coders) {
		for (i = 0; i< f->numCoders; i++) {
			free(f->coders[i].properties);
		}
		free(f->coders);
	}
	free(f->bindPairs);
	free(f->packedStreams);
	free(f->unPackSize);
}

static int
read_Folder(struct _7z_folder *f, const unsigned char *p, size_t len)
{
	const unsigned char *_p = p;
	uint64_t numInStreamsTotal = 0;
	uint64_t numOutStreamsTotal = 0;
	int r;
	unsigned i;

	memset(f, 0, sizeof(*f));

	/*
	 * Read NumCoders.
	 */
	r = parse_7zip_uint64(p, len, &(f->numCoders));
	if (r < 0)
		return (-1);
	p += r;
	len -= r;

	f->coders = calloc(f->numCoders, sizeof(*f->coders));
	if (f->coders == NULL)
		return (-1);
	for (i = 0; i< f->numCoders; i++) {
		size_t codec_size;
		int simple, attr;

		if (len == 0)
			return (-1);
		/*
		 * 0:3 CodecIdSize
		 * 4:  0 - IsSimple
		 *     1 - Is not Simple
		 * 5:  0 - No Attributes
		 *     1 - There are Attributes;
		 * 7:  Must be zero.
		 */
		codec_size = *p & 0xf;
		simple = (*p & 0x10)?0:1;
		attr = *p & 0x20;
		if (*p & 0x80)
			return (-1);/* Not supported. */
		p++;
		len--;

		/*
		 * Read Decompression Method IDs.
		 */
		if (len < codec_size)
			return (-1);

		f->coders[i].codec = decode_codec_id(p, codec_size);
		p += codec_size;
		len -= codec_size;

		if (simple) {
			f->coders[i].numInStreams = 1;
			f->coders[i].numOutStreams = 1;
		} else {
			r = parse_7zip_uint64(p, len,
			    &(f->coders[i].numInStreams));
			if (r < 0)
				return (-1);
			p += r;
			len -= r;
			r = parse_7zip_uint64(p, len,
			    &(f->coders[i].numOutStreams));
			if (r < 0)
				return (-1);
			p += r;
			len -= r;
		}

		if (attr) {
			r = parse_7zip_uint64(p, len,
				&(f->coders[i].propertiesSize));
			if (r < 0)
				return (-1);
			p += r;
			len -= r;

			if (len < f->coders[i].propertiesSize)
				return (-1);
			f->coders[i].properties =
			    malloc(f->coders[i].propertiesSize);
			if (f->coders[i].properties == NULL)
				return (-1);
			memcpy(f->coders[i].properties, p,
			    f->coders[i].propertiesSize);
			p += f->coders[i].propertiesSize;
			len -= f->coders[i].propertiesSize;
		}

		numInStreamsTotal += f->coders[i].numInStreams;
		numOutStreamsTotal += f->coders[i].numOutStreams;
	}

	if (numOutStreamsTotal == 0 ||
	    numInStreamsTotal < numOutStreamsTotal-1)
		return (-1);

	f->numBindPairs = numOutStreamsTotal - 1;
	f->bindPairs = calloc(f->numBindPairs, sizeof(*f->bindPairs));
	if (f->bindPairs == NULL)
		return (-1);
	for (i = 0; i < f->numBindPairs; i++) {
		r = parse_7zip_uint64(p, len, &(f->bindPairs[i].inIndex));
		if (r < 0)
			return (-1);
		p += r;
		len -= r;
		r = parse_7zip_uint64(p, len, &(f->bindPairs[i].outIndex));
		if (r < 0)
			return (-1);
		p += r;
		len -= r;
	}

	f->numPackedStreams = numInStreamsTotal - f->numBindPairs;
	f->packedStreams =
	    calloc(f->numPackedStreams, sizeof(*f->packedStreams));
	if (f->packedStreams == NULL)
		return (-1);
	if (f->numPackedStreams == 1) {
		for (i = 0; i < numInStreamsTotal; i++) {
			unsigned j;
			for (j = 0; j < f->numBindPairs; j++) {
				if (f->bindPairs[j].inIndex == i)
					break;
			}
			if (j == f->numBindPairs)
				break;
		}
		if (i == numInStreamsTotal)
			return (-1);
		f->packedStreams[0] = i;
	} else {
		for (i = 0; i < f->numPackedStreams; i++) {
			r = parse_7zip_uint64(p, len, &(f->packedStreams[i]));
			if (r < 0)
				return (-1);
			p += r;
			len -= r;
		}
	}
	f->numInStreams = numInStreamsTotal;
	f->numOutStreams = numOutStreamsTotal;

	return (p - _p);
}

static void
free_CodersInfo(struct _7z_coders_info *ci)
{
	unsigned i;

	if (ci->folders) {
		for (i = 0; i < ci->numFolders; i++)
			free_Folder(&(ci->folders[i]));
		free(ci->folders);
	}
}

static int
read_CodersInfo(struct _7z_coders_info *ci, const unsigned char *p, size_t len)
{
	const unsigned char *_p = p;
	struct _7z_digests digest;
	unsigned i, external;
	int r;

	memset(ci, 0, sizeof(*ci));
	memset(&digest, 0, sizeof(digest));

	if (len < 3 || *p++ != kUnPackInfo)
		goto failed;
	--len;

	if (len < 3 || *p++ != kFolder)
		goto failed;
	--len;

	/*
	 * Read NumFolders.
	 */
	r = parse_7zip_uint64(p, len, &(ci->numFolders));
	if (r < 0)
		goto failed;
	p += r;
	len -= r;

	/*
	 * Read External.
	 */
	if (len == 0)
		goto failed;
	external = *p++;
	len --;
	switch (external) {
	case 0:
		ci->folders = calloc(ci->numFolders, sizeof(*ci->folders));
		if (ci->folders == NULL)
			return (-1);
		for (i = 0; i < ci->numFolders; i++) {
			r = read_Folder(&(ci->folders[i]), p, len);
			if (r < 0)
				goto failed;
			p += r;
			len -= r;
		}
		break;
	case 1:
		r = parse_7zip_uint64(p, len, &(ci->dataStreamIndex));
		if (r < 0)
			return (r);
		p += r;
		len -= r;
		break;
	}

	if (len < 1 + ci->numFolders || *p++ != kCodersUnPackSize)
		goto failed;
	--len;

	for (i = 0; i < ci->numFolders; i++) {
		struct _7z_folder *folder = &(ci->folders[i]);
		unsigned j;

		folder->unPackSize =
		    calloc(folder->numOutStreams, sizeof(*folder->unPackSize));
		if (folder->unPackSize == NULL)
			goto failed;
		for (j = 0; j < folder->numOutStreams; j++) {
			r = parse_7zip_uint64(p, len,
			      &(folder->unPackSize[j]));
			if (r < 0)
				goto failed;
			p += r;
			len -= r;
		}
	}

	/*
	 * Read CRCs.
	 */
	if (len == 0)
		goto failed;
	if (*p == kEnd)
		return (p - _p + 1);
	if (len < 1 + ci->numFolders || *p++ != kCRC)
		goto failed;
	--len;
	r = read_Digests(&digest, ci->numFolders, p, len);
	if (r < 0)
		goto failed;
	p += r;
	len -= r;
	for (i = 0; i < ci->numFolders; i++) {
		ci->folders[i].digest_defined = digest.defineds[i];
		ci->folders[i].digest = digest.digests[i];
	}

	/*
	 *  Must be kEnd.
	 */
	if (len == 0 || *p++ != kEnd)
		goto failed;
	free_Digest(&digest);
	return (p - _p);
failed:
	free_Digest(&digest);
	return (-1);
}

static uint64_t
folder_uncompressed_size(struct _7z_folder *f)
{
	int n = f->numOutStreams;
	unsigned pairs = f->numBindPairs;

	while (--n >= 0) {
		unsigned i;
		for (i = 0; i < pairs; i++) {
			if (f->bindPairs[i].outIndex == n)
				break;
		}
		if (i >= pairs)
			return (f->unPackSize[n]);
	}
	return (0);
}

static void
free_SubStreamsInfo(struct _7z_substream_info *ss)
{
	free(ss->unpackSizes);
	free(ss->digestsDefined);
	free(ss->digests);
}

static int
read_SubStreamsInfo(struct _7z_substream_info *ss, struct _7z_folder *f,
    size_t numFolders, const unsigned char *p, size_t len)
{
	const unsigned char *_p = p;
	uint64_t *usizes;
	size_t unpack_streams;
	int r, type;
	unsigned i;
	uint32_t numDigests;

	memset(ss, 0, sizeof(*ss));

	if (len < 2 || *p++ != kSubStreamsInfo)
		return (-1);
	--len;

	for (i = 0; i < numFolders; i++)
		f[i].numUnpackStreams = 1;

	if (len < 1)
		return (-1);
	type = *p++;
	--len;

	if (type == kNumUnPackStream) {
		unpack_streams = 0;
		for (i = 0; i < numFolders; i++) {
			r = parse_7zip_uint64(p, len, &(f[i].numUnpackStreams));
			if (r < 0)
				return (-1);
			p += r;
			len -= r;
			unpack_streams += f[i].numUnpackStreams;
		}
		if (len < 1)
			return (-1);
		type = *p++;
		--len;
	} else
		unpack_streams = numFolders;

	ss->unpack_streams = unpack_streams;
	if (unpack_streams) {
		ss->unpackSizes = calloc(unpack_streams,
		    sizeof(*ss->unpackSizes));
		ss->digestsDefined = calloc(unpack_streams,
		    sizeof(*ss->digestsDefined));
		ss->digests = calloc(unpack_streams,
		    sizeof(*ss->digests));
		if (ss->unpackSizes == NULL || ss->digestsDefined == NULL ||
		    ss->digests == NULL)
			return (-1);
	}

	usizes = ss->unpackSizes;
	for (i = 0; i < numFolders; i++) {
		unsigned pack;
		uint64_t sum;

		if (f[i].numUnpackStreams == 0)
			continue;

		sum = 0;
		if (type == kSize) {
			for (pack = 1; pack < f[i].numUnpackStreams; pack++) {
				r = parse_7zip_uint64(p, len, usizes);
				if (r < 0)
					return (-1);
				p += r;
				len -= r;
				sum += *usizes++;
			}
		}
		*usizes++ = folder_uncompressed_size(&f[i]) - sum;
	}

	if (type == kSize) {
		if (len < 1)
			return (-1);
		type = *p++;
		--len;
	}

	for (i = 0; i < unpack_streams; i++) {
		ss->digestsDefined[i] = 0;
		ss->digests[i] = 0;
	}

	numDigests = 0;
	for (i = 0; i < numFolders; i++) {
		if (f[i].numUnpackStreams != 1 ||
			!f[i].digest_defined)
			numDigests += f[i].numUnpackStreams;
	}

	if (type == kCRC) {
		struct _7z_digests tmpDigests;
		unsigned char *digestsDefined = ss->digestsDefined;
		uint32_t * digests = ss->digests;
		int di = 0;

		memset(&tmpDigests, 0, sizeof(tmpDigests));
		r = read_Digests(&(tmpDigests), numDigests, p, len);
		if (r < 0) {
			free_Digest(&tmpDigests);
			return (-1);
		}
		p += r;
		len -= r;
		for (i = 0; i < numFolders; i++) {
			if (f[i].numUnpackStreams == 1 && f[i].digest_defined) {
				*digestsDefined++ = 1;
				*digests++ = f[i].digest;
			} else {
				unsigned j;

				for (j = 0; j < f[i].numUnpackStreams;
				    j++, di++) {
					*digestsDefined++ =
					    tmpDigests.defineds[di];
					*digests++ =
					    tmpDigests.digests[di];
				}
			}
		}
		free_Digest(&tmpDigests);
		if (len < 1)
			return (-1);
		type = *p++;
		--len;
	}

	/*
	 *  Must be kEnd.
	 */
	if (type != kEnd)
		return (-1);
	return (p - _p);
}

static void
free_StreamsInfo(struct _7z_stream_info *si)
{
	free_PackInfo(&(si->pi));
	free_CodersInfo(&(si->ci));
	free_SubStreamsInfo(&(si->ss));
}

static int
read_StreamsInfo(struct _7zip *zip, struct _7z_stream_info *si,
    const unsigned char *p, size_t len)
{
	const unsigned char *_p = p;
	unsigned i;
	int r;

	memset(si, 0, sizeof(*si));

	if (len > 0 && *p == kPackInfo) {
		uint64_t packPos;

		r = read_PackInfo(&(si->pi), p, len);
		if (r < 0)
			return (-1);
		p += r;
		len -= r;

		/*
		 * Calculate packed stream positions.
		 */
		packPos = si->pi.pos;
		for (i = 0; i < si->pi.numPackStreams; i++) {
			si->pi.positions[i] = packPos;
			packPos += si->pi.sizes[i];
			if (packPos > zip->header_offset)
				return (-1);
		}
	}
	if (len > 0 && *p == kUnPackInfo) {
		uint32_t packIndex;
		struct _7z_folder *f;

		r = read_CodersInfo(&(si->ci), p, len);
		if (r < 0)
			return (-1);
		p += r;
		len -= r;

		/*
		 * Calculate packed stream indexes.
		 */
		packIndex = 0;
		f = si->ci.folders;
		for (i = 0; i < si->ci.numFolders; i++) {
			f[i].packIndex = packIndex;
			packIndex += f[i].numPackedStreams;
			if (packIndex > si->pi.numPackStreams)
				return (-1);
		}
	}
	if (len > 0 && *p == kSubStreamsInfo) {
		r = read_SubStreamsInfo(&(si->ss),
		    si->ci.folders, si->ci.numFolders, p, len);
		if (r < 0)
			return (-1);
		p += r;
		len -= r;
	}

	/*
	 *  Must be kEnd.
	 */
	if (len == 0 || *p++ != kEnd)
		return (-1);
	return (p - _p);
}

static void
free_Header(struct _7z_header_info *h)
{
	free(h->emptyStreamBools);
	free(h->emptyFileBools);
	free(h->antiBools);
	free(h->attrBools);
}

static int
read_Header(struct _7zip *zip, struct _7z_header_info *h,
    const unsigned char *p, size_t len)
{
	const unsigned char *_p = p;
	struct _7z_folder *folders;
	struct _7z_stream_info *si = &(zip->si);
	struct _7zip_entry *entries;
	uint32_t folderIndex, indexInFolder;
	unsigned i;
	int eindex, empty_streams, r, sindex;

	if (len < 2 || *p++ != kHeader)
		return (-1);
	len--;

	/*
	 * Read ArchiveProperties.
	 */
	if (*p == kArchiveProperties) {
		p++;
		len--;

		for (;;) {
			uint64_t size;
			int atype = *p++;
			len--;
			if (atype == 0)
				break;
			r = parse_7zip_uint64(p, len, &size);
			if (r < 0 || len < r + size)
				return (-1);
			p += r + size;
			len -= r + size;
		}
	}

	/*
	 * Read MainStreamsInfo.
	 */
	if (*p == kMainStreamsInfo) {
		p++;
		len--;
		r = read_StreamsInfo(zip, &(zip->si), p, len);
		if (r < 0)
			return (-1);
		p += r;
		len -= r;
	}
	if (len == 0)
		return (-1);
	if (*p == kEnd)
		return (p - _p + 1);

	/*
	 * Read FilesInfo.
	 */
	if (len < 2 || *p++ != kFilesInfo)
		return (-1);
	len--;

	r = parse_7zip_uint64(p, len, &(zip->numFiles));
	if (r < 0)
		return (-1);
	p += r;
	len -= r;

	zip->entries = calloc(zip->numFiles, sizeof(*zip->entries));
	if (zip->entries == NULL)
		return (-1);
	entries = zip->entries;

	empty_streams = 0;
	for (;;) {
		int type;
		uint64_t size;
		size_t ll;

		if (len < 1)
			return (-1);
		type = *p++;
		len--;
		if (type == kEnd)
			break;

		r = parse_7zip_uint64(p, len, &size);
		if (r < 0 || len < size)
			return (-1);
		p += r;
		len -= r;
		ll = (size_t)size;
		len -= ll;

		switch (type) {
		case kEmptyStream:
			h->emptyStreamBools = calloc(zip->numFiles,
			    sizeof(*h->emptyStreamBools));
			if (h->emptyStreamBools == NULL)
				return (-1);
			r = read_Bools(h->emptyStreamBools, zip->numFiles,
			    p, ll);
			if (r < 0)
				return (-1);
			p += r;
			ll -= r;
			empty_streams = 0;
			for (i = 0; i < zip->numFiles; i++) {
				if (h->emptyStreamBools[i])
					empty_streams++;
			}
			break;
		case kEmptyFile:
			h->emptyFileBools = calloc(empty_streams,
			    sizeof(*h->emptyFileBools));
			if (h->emptyFileBools == NULL)
				return (-1);
			r = read_Bools(h->emptyFileBools, empty_streams,
			    p, len);
			if (r < 0)
				return (-1);
			p += r;
			ll -= r;
			break;
		case kAnti:
			h->antiBools = calloc(empty_streams,
			    sizeof(*h->antiBools));
			if (h->antiBools == NULL)
				return (-1);
			r = read_Bools(h->antiBools, empty_streams, p, len);
			if (r < 0)
				return (-1);
			p += r;
			ll -= r;
			break;
		case kCTime:
		case kATime:
		case kMTime:
			r = read_Times(zip, h, type, p, ll);
			if (r < 0)
				return (-1);
			p += r;
			ll -= r;
			break;
		case kName:
		{
			unsigned char *np;
			size_t nl;

			if (ll < 1)
				return (-1);
			p++; ll--;/* Skip one byte. */
			if ((ll & 1) || ll < zip->numFiles * 4)
				return (-1);

			zip->entry_names = malloc(ll);
			if (zip->entry_names == NULL)
				return (-1);
			memcpy(zip->entry_names, p, ll);
			np = zip->entry_names;
			nl = ll;

			for (i = 0; i < zip->numFiles; i++) {
				entries[i].utf16name = np;
#if defined(_WIN32) && !defined(__CYGWIN__) && defined(_DEBUG)
				entries[i].wname = (wchar_t *)np;
#endif

				/* Find a terminator. */
				while (nl >= 2 && (np[0] || np[1])) {
					np += 2;
					nl -= 2;
				}
				if (nl < 2)
					return (-1);/* Terminator not found */
				entries[i].name_len = np - entries[i].utf16name;
				np += 2;
				nl -= 2;
			}
			break;
		}
		case kAttributes:
		{
			int allAreDefined;

			if (ll < 2)
				return (-1);
			allAreDefined = *p++;
			--ll;
			p++; --ll;/* Skip one byte. */
			h->attrBools = calloc(zip->numFiles,
			    sizeof(*h->attrBools));
			if (h->attrBools == NULL)
				return (-1);
			if (allAreDefined)
				memset(h->attrBools, 1, zip->numFiles);
			else {
				r = read_Bools(h->attrBools,
				      zip->numFiles, p, ll);
				if (r < 0)
					return (-1);
				p += r;
				ll -= r;
			}
			for (i = 0; i < zip->numFiles; i++) {
				if (h->attrBools[i]) {
					if (ll < 4)
						return (-1);
					entries[i].attr = archive_le32dec(p);
					p += 4;
					ll -= 4;
				}
			}
			break;
		}
		default:
			break;
		}
		/* Skip remaining data. */
		p += ll;
	}

	/*
	 * Set up entry's attributes.
	 */
	folders = si->ci.folders;
	eindex = sindex = 0;
	folderIndex = indexInFolder = 0;
	for (i = 0; i < zip->numFiles; i++) {
		if (h->emptyStreamBools == NULL ||
		    h->emptyStreamBools[i] == 0)
			entries[i].flg |= HAS_STREAM;
		/* The high 16 bits of attributes is a posix file mode. */
		entries[i].mode = entries[i].attr >> 16;
		if (entries[i].flg & HAS_STREAM) {
			if ((size_t)sindex >= si->ss.unpack_streams)
				return (-1);
			if (entries[i].mode == 0)
				entries[i].mode = AE_IFREG | 0777;
			if (si->ss.digestsDefined[sindex])
				entries[i].flg |= CRC32_IS_SET;
			entries[i].ssIndex = sindex;
			sindex++;
		} else {
			int dir;
			if (h->emptyFileBools == NULL)
				dir = 1;
			else {
				if (h->emptyFileBools[eindex])
					dir = 0;
				else
					dir = 1;
				eindex++;
			}
			if (entries[i].mode == 0) {
				if (dir)
					entries[i].mode = AE_IFDIR | 0777;
				else
					entries[i].mode = AE_IFREG | 0777;
			} else if (dir &&
			    (entries[i].mode & AE_IFMT) != AE_IFDIR) {
				entries[i].mode &= ~AE_IFMT;
				entries[i].mode |= AE_IFDIR;
			}
			if ((entries[i].mode & AE_IFMT) == AE_IFDIR &&
			    entries[i].name_len >= 2 &&
			    (entries[i].utf16name[entries[i].name_len-2] != '/' ||
			     entries[i].utf16name[entries[i].name_len-1] != 0)) {
				entries[i].utf16name[entries[i].name_len] = '/';
				entries[i].utf16name[entries[i].name_len+1] = 0;
				entries[i].name_len += 2;
			}
			entries[i].ssIndex = -1;
		}
		if (entries[i].attr & 0x01)
			entries[i].mode &= ~0222;/* Read only. */

		if ((entries[i].flg & HAS_STREAM) == 0 && indexInFolder == 0) {
			/*
			 * The entry is an empty file or a directory file,
			 * those both have no contents.
			 */
			entries[i].folderIndex = -1;
			continue;
		}
		if (indexInFolder == 0) {
			for (;;) {
				if (folderIndex >= si->ci.numFolders)
					return (-1);
				if (folders[folderIndex].numUnpackStreams)
					break;
				folderIndex++;
			}
		}
		entries[i].folderIndex = folderIndex;
		if ((entries[i].flg & HAS_STREAM) == 0)
			continue;
		indexInFolder++;
		if (indexInFolder >= folders[folderIndex].numUnpackStreams) {
			folderIndex++;
			indexInFolder = 0;
		}
	}

	return (p - _p);
}

#define EPOC_TIME ARCHIVE_LITERAL_ULL(116444736000000000)
static void
fileTimeToUtc(uint64_t fileTime, time_t *time, long *ns)
{

	if (fileTime >= EPOC_TIME) {
		fileTime -= EPOC_TIME;
		/* milli seconds base */
		*time = (time_t)(fileTime / 10000000);
		/* nano seconds base */
		*ns = (long)(fileTime % 10000000) * 100;
	} else {
		*time = 0;
		*ns = 0;
	}
}

static int
read_Times(struct _7zip *zip, struct _7z_header_info *h, int type,
    const unsigned char *p, size_t len)
{
	const unsigned char *_p = p;
	struct _7zip_entry *entries = zip->entries;
	unsigned char *timeBools;
	int r;
	int allAreDefined, external;
	unsigned i;

	timeBools = calloc(zip->numFiles, sizeof(*timeBools));
	if (timeBools == NULL)
		return (-1);

	if (len < 1)
		goto failed;
	allAreDefined = *p++;
	len--;
	if (allAreDefined)
		memset(timeBools, 1, zip->numFiles);
	else {
		r = read_Bools(timeBools, zip->numFiles, p, len);
		if (r < 0)
			goto failed;
		p += r;
		len -= r;
	}

	if (len < 1)
		goto failed;
	external = *p++;
	len--;
	if (external) {
		r = parse_7zip_uint64(p, len, &(h->dataIndex));
		if (r < 0)
			goto failed;
		p += r;
		len -= r;
	}

	for (i = 0; i < zip->numFiles; i++) {
		if (!timeBools[i])
			continue;
		if (len < 8)
			goto failed;
		switch (type) {
		case kCTime:
			fileTimeToUtc(archive_le64dec(p),
			    &(entries[i].ctime),
			    &(entries[i].ctime_ns));
			entries[i].flg |= CTIME_IS_SET;
			break;
		case kATime:
			fileTimeToUtc(archive_le64dec(p),
			    &(entries[i].atime),
			    &(entries[i].atime_ns));
			entries[i].flg |= ATIME_IS_SET;
			break;
		case kMTime:
			fileTimeToUtc(archive_le64dec(p),
			    &(entries[i].mtime),
			    &(entries[i].mtime_ns));
			entries[i].flg |= MTIME_IS_SET;
			break;
		}
		p += 8;
		len -= 8;
	}

	free(timeBools);
	return (p - _p);
failed:
	free(timeBools);
	return (-1);
}

static ssize_t
decode_header_image(struct archive_read *a, struct _7zip *zip,
    struct _7z_stream_info *si, const unsigned char *p, uint64_t len,
    const void **image)
{
	const unsigned char *v;
	size_t vsize;
	int r;

	errno = 0;
	r = read_StreamsInfo(zip, si, p, len);
	if (r < 0) {
		if (errno == ENOMEM)
			archive_set_error(&a->archive, -1,
			    "Couldn't allocate memory");
		else
			archive_set_error(&a->archive, -1,
			    "Malformed 7-Zip archive");
		return (ARCHIVE_FATAL);
	}

	if (si->pi.numPackStreams == 0 || si->ci.numFolders == 0) {
		archive_set_error(&a->archive, -1, "Malformed 7-Zip archive");
		return (ARCHIVE_FATAL);
	}

	if (zip->header_offset < si->pi.pos + si->pi.sizes[0] ||
	    (int64_t)(si->pi.pos + si->pi.sizes[0]) < 0 ||
	    si->pi.sizes[0] == 0 || (int64_t)si->pi.pos < 0) {
		archive_set_error(&a->archive, -1, "Malformed Header offset");
		return (ARCHIVE_FATAL);
	}

	r = setup_decode_folder(a, si->ci.folders, 1);
	if (r != ARCHIVE_OK)
		return (ARCHIVE_FATAL);

	/* Get an uncompressed header size. */
	vsize = (size_t)zip->folder_outbytes_remaining;

	/*
	 * Allocate an uncompressed buffer for the header image.
	 */
	zip->uncompressed_buffer_size = 64 * 1024;
	if (vsize > zip->uncompressed_buffer_size)
		zip->uncompressed_buffer_size = vsize;
	zip->uncompressed_buffer = malloc(zip->uncompressed_buffer_size);
	if (zip->uncompressed_buffer == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "No memory for 7-Zip decompression");
		return (ARCHIVE_FATAL);
	}

	/* Get the bytes we can read to decode the header. */
	zip->pack_stream_inbytes_remaining = si->pi.sizes[0];

	/* Seek the read point. */
	if (__archive_read_seek(a, si->pi.pos + zip->seek_base, SEEK_SET) < 0)
		return (ARCHIVE_FATAL);
	zip->header_offset = si->pi.pos;

	/* Extract a pack stream. */
	r = extract_pack_stream(a);
	if (r < 0)
		return (r);
	for (;;) {
		ssize_t bytes;
		
		bytes = get_uncompressed_data(a, image, vsize);
		if (bytes < 0)
			return (r);
		if (bytes != vsize) {
			if (*image != zip->uncompressed_buffer) {
				/* This might happen if the coder was COPY.
				 * We have to make sure we read a full plain
				 * header image. */
				if (NULL==__archive_read_ahead(a, vsize, NULL))
					return (ARCHIVE_FATAL);
				continue;
			} else {
				archive_set_error(&a->archive, -1,
				    "Malformed 7-Zip archive file");
				return (ARCHIVE_FATAL);
			}
		}
		break;
	}
	v = *image;

	/* Clean up variables which will not be used for decoding the
	 * archive header */
	zip->pack_stream_remaining = 0;
	zip->pack_stream_index = 0;
	zip->folder_outbytes_remaining = 0;
	zip->uncompressed_buffer_bytes_remaining = 0;
	zip->pack_stream_bytes_unconsumed = 0;

	/* Check the header CRC. */
	if (si->ci.folders[0].digest_defined){
		uint32_t c = crc32(0, v, vsize);
		if (c != si->ci.folders[0].digest) {
			archive_set_error(&a->archive, -1, "Header CRC error");
			return (ARCHIVE_FATAL);
		}
	}
	return ((ssize_t)vsize);
}

static int
slurp_central_directory(struct archive_read *a, struct _7zip *zip,
    struct _7z_header_info *header)
{
	const unsigned char *p;
	const void *image;
	uint64_t len;
	uint64_t next_header_offset;
	uint64_t next_header_size;
	uint32_t next_header_crc;
	ssize_t bytes_avail, image_bytes;
	int r;

	if ((p = __archive_read_ahead(a, 32, &bytes_avail)) == NULL)
		return (ARCHIVE_FATAL);

	if ((p[0] == 'M' && p[1] == 'Z') || memcmp(p, "\x7F\x45LF", 4) == 0) {
		/* This is an executable ? Must be self-extracting... */
		r = skip_sfx(a, bytes_avail);
		if (r < ARCHIVE_WARN)
			return (r);
		if ((p = __archive_read_ahead(a, 32, NULL)) == NULL)
			return (ARCHIVE_FATAL);
	}
	zip->seek_base += 32;

	if (memcmp(p, _7ZIP_SIGNATURE, 6) != 0) {
		archive_set_error(&a->archive, -1, "Not 7-Zip archive file");
		return (ARCHIVE_FATAL);
	}

	/* CRC check. */
	if (crc32(0, (unsigned char *)p + 12, 20) != archive_le32dec(p + 8)) {
		archive_set_error(&a->archive, -1, "Header CRC error");
		return (ARCHIVE_FATAL);
	}

	next_header_offset = archive_le64dec(p + 12);
	next_header_size = archive_le64dec(p + 20);
	next_header_crc = archive_le32dec(p + 28);

	if (next_header_size == 0)
		/* There is no entry in an archive file. */
		return (ARCHIVE_EOF);

	if (((int64_t)next_header_offset) < 0) {
		archive_set_error(&a->archive, -1, "Malformed 7-Zip archive");
		return (ARCHIVE_FATAL);
	}
	if (__archive_read_seek(a, next_header_offset + zip->seek_base,
	    SEEK_SET) < 0)
		return (ARCHIVE_FATAL);
	zip->header_offset = next_header_offset;

	if ((p = __archive_read_ahead(a, next_header_size, NULL)) == NULL)
		return (ARCHIVE_FATAL);

	if (crc32(0, p, next_header_size) != next_header_crc) {
		archive_set_error(&a->archive, -1, "Damaged 7-Zip archive");
		return (ARCHIVE_FATAL);
	}

	len = next_header_size;
	/* Parse ArchiveProperties. */
	switch (p[0]) {
	case kEncodedHeader:
		p++;
		len--;

		/*
		 * The archive has an encoded header and we have to decode it
		 * in order to parse the header correctly.
		 */
		image_bytes =
		    decode_header_image(a, zip, &(zip->si), p, len, &image);
		free_StreamsInfo(&(zip->si));
		memset(&(zip->si), 0, sizeof(zip->si));
		if (image_bytes < 0)
			return (ARCHIVE_FATAL);
		p = image;
		len = image_bytes;
		/* FALL THROUGH */
	case kHeader:
		/*
		 * Parse the header.
		 */
		errno = 0;
		r = read_Header(zip, header, p, len);
		if (r < 0) {
			if (errno == ENOMEM)
				archive_set_error(&a->archive, -1,
				    "Couldn't allocate memory");
			else
				archive_set_error(&a->archive, -1,
				    "Damaged 7-Zip archive");
			return (ARCHIVE_FATAL);
		}
		if (len - r == 0 || p[r] != kEnd) {
			archive_set_error(&a->archive, -1,
			    "Malformed 7-Zip archive");
			return (ARCHIVE_FATAL);
		}
		break;
	default:
		archive_set_error(&a->archive, -1,
		    "Unexpected Property ID = %X", p[0]);
		return (ARCHIVE_FATAL);
	}
	zip->stream_offset = -1;

	/*
	 * If the uncompressed buffer was allocated more than 64K for
	 * the header image, release it.
	 */
	if (zip->uncompressed_buffer != NULL &&
	    zip->uncompressed_buffer_size != 64 * 1024) {
		free(zip->uncompressed_buffer);
		zip->uncompressed_buffer = NULL;
		zip->uncompressed_buffer_size = 0;
	}

	return (ARCHIVE_OK);
}

static ssize_t
get_uncompressed_data(struct archive_read *a, const void **buff, size_t size)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	ssize_t bytes_avail;

	if (zip->codec == _7Z_COPY && zip->codec2 == -1) {
		/* Copy mode. */

		/*
		 * Note: '1' here is a performance optimization.
		 * Recall that the decompression layer returns a count of
		 * available bytes; asking for more than that forces the
		 * decompressor to combine reads by copying data.
		 */
		*buff = __archive_read_ahead(a, 1, &bytes_avail);
		if (bytes_avail <= 0) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated 7-Zip file data");
			return (ARCHIVE_FATAL);
		}
		if ((size_t)bytes_avail >
		    zip->uncompressed_buffer_bytes_remaining)
			bytes_avail = (ssize_t)
			    zip->uncompressed_buffer_bytes_remaining;
		if ((size_t)bytes_avail > size)
			bytes_avail = (ssize_t)size;

		zip->pack_stream_bytes_unconsumed = bytes_avail;
	} else if (zip->uncompressed_buffer_pointer == NULL) {
		/* Decompression has failed. */
		archive_set_error(&(a->archive),
		    ARCHIVE_ERRNO_MISC, "Damaged 7-Zip archive");
		return (ARCHIVE_FATAL);
	} else {
		/* Packed mode. */
		if (size > zip->uncompressed_buffer_bytes_remaining)
			bytes_avail = (ssize_t)
			    zip->uncompressed_buffer_bytes_remaining;
		else
			bytes_avail = (ssize_t)size;
		*buff = zip->uncompressed_buffer_pointer;
		zip->uncompressed_buffer_pointer += bytes_avail;
	}
	zip->uncompressed_buffer_bytes_remaining -= bytes_avail;
	return (bytes_avail);
}

static ssize_t
extract_pack_stream(struct archive_read *a)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	ssize_t bytes_avail;
	int r;

	if (zip->codec == _7Z_COPY && zip->codec2 == -1) {
		if (__archive_read_ahead(a, 1, &bytes_avail) == NULL
		    || bytes_avail <= 0) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated 7-Zip file body");
			return (ARCHIVE_FATAL);
		}
		if (bytes_avail > zip->pack_stream_inbytes_remaining)
			bytes_avail = zip->pack_stream_inbytes_remaining;
		zip->pack_stream_inbytes_remaining -= bytes_avail;
		if (bytes_avail > zip->folder_outbytes_remaining)
			bytes_avail = zip->folder_outbytes_remaining;
		zip->folder_outbytes_remaining -= bytes_avail;
		zip->uncompressed_buffer_bytes_remaining = bytes_avail;
		return (ARCHIVE_OK);
	}

	/* If the buffer hasn't been allocated, allocate it now. */
	if (zip->uncompressed_buffer == NULL) {
		zip->uncompressed_buffer_size = 64 * 1024;
		zip->uncompressed_buffer =
		    malloc(zip->uncompressed_buffer_size);
		if (zip->uncompressed_buffer == NULL) {
			archive_set_error(&a->archive, ENOMEM,
			    "No memory for 7-Zip decompression");
			return (ARCHIVE_FATAL);
		}
	}
	zip->uncompressed_buffer_bytes_remaining = 0;
	zip->uncompressed_buffer_pointer = NULL;
	for (;;) {
		size_t bytes_in, bytes_out;
		const void *buff_in;
		unsigned char *buff_out;
		int eof;

		/*
		 * Note: '1' here is a performance optimization.
		 * Recall that the decompression layer returns a count of
		 * available bytes; asking for more than that forces the
		 * decompressor to combine reads by copying data.
		 */
		buff_in = __archive_read_ahead(a, 1, &bytes_avail);
		if (bytes_avail <= 0) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated 7-Zip file body");
			return (ARCHIVE_FATAL);
		}

		buff_out = zip->uncompressed_buffer
			+ zip->uncompressed_buffer_bytes_remaining;
		bytes_out = zip->uncompressed_buffer_size
			- zip->uncompressed_buffer_bytes_remaining;
		bytes_in = bytes_avail;
		if (bytes_in > zip->pack_stream_inbytes_remaining)
			bytes_in = zip->pack_stream_inbytes_remaining;
		/* Drive decompression. */
		r = decompress(a, zip, buff_out, &bytes_out,
			buff_in, &bytes_in);
		switch (r) {
		case ARCHIVE_OK:
			eof = 0;
			break;
		case ARCHIVE_EOF:
			eof = 1;
			break;
		default:
			return (ARCHIVE_FATAL);
		}
		zip->pack_stream_inbytes_remaining -= bytes_in;
		if (bytes_out > zip->folder_outbytes_remaining)
			bytes_out = zip->folder_outbytes_remaining;
		zip->folder_outbytes_remaining -= bytes_out;
		zip->uncompressed_buffer_bytes_remaining += bytes_out;
		zip->pack_stream_bytes_unconsumed = bytes_in;

		/*
		 * Continue decompression until uncompressed_buffer is full.
		 */
		if (zip->uncompressed_buffer_bytes_remaining ==
		    zip->uncompressed_buffer_size)
			break;
		if (zip->pack_stream_inbytes_remaining == 0 &&
		    zip->folder_outbytes_remaining == 0)
			break;
		if (eof || (bytes_in == 0 && bytes_out == 0)) {
			archive_set_error(&(a->archive),
			    ARCHIVE_ERRNO_MISC, "Damaged 7-Zip archive");
			return (ARCHIVE_FATAL);
		}
		read_consume(a);
	}
	zip->uncompressed_buffer_pointer = zip->uncompressed_buffer;
	return (ARCHIVE_OK);
}

static int
seek_pack(struct archive_read *a)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	uint64_t pack_offset;

	if (zip->pack_stream_remaining <= 0) {
		archive_set_error(&(a->archive),
		    ARCHIVE_ERRNO_MISC, "Damaged 7-Zip archive");
		return (ARCHIVE_FATAL);
	}
	zip->pack_stream_inbytes_remaining =
	    zip->si.pi.sizes[zip->pack_stream_index];
	pack_offset = zip->si.pi.positions[zip->pack_stream_index];
	if (zip->stream_offset != pack_offset) {
		if (0 > __archive_read_seek(a, pack_offset + zip->seek_base,
		    SEEK_SET))
			return (ARCHIVE_FATAL);
		zip->stream_offset = pack_offset;
	}
	zip->pack_stream_index++;
	zip->pack_stream_remaining--;
	return (ARCHIVE_OK);
}

static ssize_t
read_stream(struct archive_read *a, const void **buff, size_t size)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	uint64_t skip_bytes = 0;
	int r;

	if (zip->uncompressed_buffer_bytes_remaining == 0) {
		if (zip->pack_stream_inbytes_remaining > 0) {
			r = extract_pack_stream(a);
			if (r < 0)
				return (r);
			return (get_uncompressed_data(a, buff, size));
		} else if (zip->folder_outbytes_remaining > 0) {
			/* Extract a remaining pack stream. */
			r = extract_pack_stream(a);
			if (r < 0)
				return (r);
			return (get_uncompressed_data(a, buff, size));
		}
	} else
		return (get_uncompressed_data(a, buff, size));

	/*
	 * Current pack stream has been consumed.
	 */
	if (zip->pack_stream_remaining == 0) {
		/*
		 * All current folder's pack streams have been
		 * consumed. Switch to next folder.
		 */

		if (zip->folder_index == 0 &&
		    (zip->si.ci.folders[zip->entry->folderIndex].skipped_bytes
		     || zip->folder_index != zip->entry->folderIndex)) {
			zip->folder_index = zip->entry->folderIndex;
			skip_bytes =
			    zip->si.ci.folders[zip->folder_index].skipped_bytes;
		}

		if (zip->folder_index >= zip->si.ci.numFolders) {
			/*
			 * We have consumed all folders and its pack streams.
			 */
			*buff = NULL;
			return (0);
		}
		r = setup_decode_folder(a,
			&(zip->si.ci.folders[zip->folder_index]), 0);
		if (r != ARCHIVE_OK)
			return (ARCHIVE_FATAL);

		zip->folder_index++;
	}

	/*
	 * Switch to next pack stream.
	 */
	r = seek_pack(a);
	if (r < 0)
		return (r);

	/* Extract a new pack stream. */
	r = extract_pack_stream(a);
	if (r < 0)
		return (r);

	/*
	 * Skip the bytes we alrady has skipped in skip_stream(). 
	 */
	while (skip_bytes) {
		ssize_t skipped;

		if (zip->uncompressed_buffer_bytes_remaining == 0) {
			if (zip->pack_stream_inbytes_remaining > 0) {
				r = extract_pack_stream(a);
				if (r < 0)
					return (r);
			} else if (zip->folder_outbytes_remaining > 0) {
				/* Extract a remaining pack stream. */
				r = extract_pack_stream(a);
				if (r < 0)
					return (r);
			} else {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_FILE_FORMAT,
				    "Truncated 7-Zip file body");
				return (ARCHIVE_FATAL);
			}
		}
		skipped = get_uncompressed_data(a, buff, skip_bytes);
		if (skipped < 0)
			return (skipped);
		skip_bytes -= skipped;
		if (zip->pack_stream_bytes_unconsumed)
			read_consume(a);
	}

	return (get_uncompressed_data(a, buff, size));
}

static int
setup_decode_folder(struct archive_read *a, struct _7z_folder *folder,
    int header)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	const struct _7z_coder *coder1, *coder2;
	const char *cname = (header)?"archive header":"file content";
	unsigned i;
	int r, found_bcj2 = 0;

	/*
	 * Release the memory which the previous folder used for BCJ2.
	 */
	for (i = 0; i < 3; i++) {
		if (zip->sub_stream_buff[i] != NULL)
			free(zip->sub_stream_buff[i]);
		zip->sub_stream_buff[i] = NULL;
	}

	/*
	 * Initialize a stream reader.
	 */
	zip->pack_stream_remaining = (unsigned)folder->numPackedStreams;
	zip->pack_stream_index = (unsigned)folder->packIndex;
	zip->folder_outbytes_remaining = folder_uncompressed_size(folder);
	zip->uncompressed_buffer_bytes_remaining = 0;

	/*
	 * Check coder types.
	 */
	for (i = 0; i < folder->numCoders; i++) {
		if (folder->coders[i].codec == _7Z_CRYPTO) {
			archive_set_error(&(a->archive),
			    ARCHIVE_ERRNO_MISC,
			    "The %s is encrypted, "
			    "but currently not supported", cname);
			return (ARCHIVE_FATAL);
		}
		if (folder->coders[i].codec == _7Z_X86_BCJ2)
			found_bcj2++;
	}
	if ((folder->numCoders > 2 && !found_bcj2) || found_bcj2 > 1) {
		archive_set_error(&(a->archive),
		    ARCHIVE_ERRNO_MISC,
		    "The %s is encoded with many filters, "
		    "but currently not supported", cname);
		return (ARCHIVE_FATAL);
	}
	coder1 = &(folder->coders[0]);
	if (folder->numCoders == 2)
		coder2 = &(folder->coders[1]);
	else
		coder2 = NULL;

	if (found_bcj2) {
		/*
		 * Preparation to decode BCJ2.
		 * Decoding BCJ2 requires four sources. Those are at least,
		 * as far as I know, two types of the storage form.
		 */
		const struct _7z_coder *fc = folder->coders;
		static const struct _7z_coder coder_copy = {0, 1, 1, 0, NULL};
		const struct _7z_coder *scoder[3] =
			{&coder_copy, &coder_copy, &coder_copy};
		const void *buff;
		ssize_t bytes;
		unsigned char *b[3] = {NULL, NULL, NULL};
		uint64_t sunpack[3] ={-1, -1, -1};
		size_t s[3] = {0, 0, 0};
		int idx[3] = {0, 1, 2};

		if (folder->numCoders == 4 && fc[3].codec == _7Z_X86_BCJ2 &&
		    folder->numInStreams == 7 && folder->numOutStreams == 4 &&
		    zip->pack_stream_remaining == 4) {
			/* Source type 1 made by 7zr or 7z with -m options. */
			if (folder->bindPairs[0].inIndex == 5) {
				/* The form made by 7zr */
				idx[0] = 1; idx[1] = 2; idx[2] = 0;
				scoder[1] = &(fc[1]);
				scoder[2] = &(fc[0]);
				sunpack[1] = folder->unPackSize[1];
				sunpack[2] = folder->unPackSize[0];
				coder1 = &(fc[2]);
			} else {
				/*
				 * NOTE: Some patterns do not work.
				 * work:
				 *  7z a -m0=BCJ2 -m1=COPY -m2=COPY
				 *       -m3=(any)
				 *  7z a -m0=BCJ2 -m1=COPY -m2=(any)
				 *       -m3=COPY
				 *  7z a -m0=BCJ2 -m1=(any) -m2=COPY
				 *       -m3=COPY
				 * not work:
				 *  other patterns.
				 *
				 * We have to handle this like `pipe' or
				 * our libarchive7s filter frame work,
				 * decoding the BCJ2 main stream sequentially,
				 * m3 -> m2 -> m1 -> BCJ2.
				 *
				 */
				if (fc[0].codec == _7Z_COPY &&
				    fc[1].codec == _7Z_COPY)
					coder1 = &(folder->coders[2]);
				else if (fc[0].codec == _7Z_COPY &&
				    fc[2].codec == _7Z_COPY)
					coder1 = &(folder->coders[1]);
				else if (fc[1].codec == _7Z_COPY &&
				    fc[2].codec == _7Z_COPY)
					coder1 = &(folder->coders[0]);
				else {
					archive_set_error(&(a->archive),
					    ARCHIVE_ERRNO_MISC,
					    "Unsupported form of "
					    "BCJ2 streams");
					return (ARCHIVE_FATAL);
				}
			}
			coder2 = &(fc[3]);
			zip->main_stream_bytes_remaining =
				folder->unPackSize[2];
		} else if (coder2 != NULL && coder2->codec == _7Z_X86_BCJ2 &&
		    zip->pack_stream_remaining == 4 &&
		    folder->numInStreams == 5 && folder->numOutStreams == 2) {
			/* Source type 0 made by 7z */
			zip->main_stream_bytes_remaining =
				folder->unPackSize[0];
		} else {
			/* We got an unexpected form. */
			archive_set_error(&(a->archive),
			    ARCHIVE_ERRNO_MISC,
			    "Unsupported form of BCJ2 streams");
			return (ARCHIVE_FATAL);
		}

		/* Skip the main stream at this time. */
		if ((r = seek_pack(a)) < 0)
			return (r);
		zip->pack_stream_bytes_unconsumed =
		    zip->pack_stream_inbytes_remaining;
		read_consume(a);

		/* Read following three sub streams. */
		for (i = 0; i < 3; i++) {
			const struct _7z_coder *coder = scoder[i];

			if ((r = seek_pack(a)) < 0)
				return (r);

			if (sunpack[i] == -1)
				zip->folder_outbytes_remaining =
				    zip->pack_stream_inbytes_remaining;
			else
				zip->folder_outbytes_remaining = sunpack[i];

			r = init_decompression(a, zip, coder, NULL);
			if (r != ARCHIVE_OK)
				return (ARCHIVE_FATAL);

			/* Allocate memory for the decorded data of a sub
			 * stream. */
			b[i] = malloc(zip->folder_outbytes_remaining);
			if (b[i] == NULL) {
				archive_set_error(&a->archive, ENOMEM,
				    "No memory for 7-Zip decompression");
				return (ARCHIVE_FATAL);
			}

			/* Extract a sub stream. */
			while (zip->pack_stream_inbytes_remaining > 0) {
				r = extract_pack_stream(a);
				if (r < 0)
					return (r);
				bytes = get_uncompressed_data(a, &buff,
				    zip->uncompressed_buffer_bytes_remaining);
				if (bytes < 0)
					return ((int)bytes);
				memcpy(b[i]+s[i], buff, bytes);
				s[i] += bytes;
				if (zip->pack_stream_bytes_unconsumed)
					read_consume(a);
			}
		}

		/* Set the sub streams to the right place. */
		for (i = 0; i < 3; i++) {
			zip->sub_stream_buff[i] = b[idx[i]];
			zip->sub_stream_size[i] = s[idx[i]];
			zip->sub_stream_bytes_remaining[i] = s[idx[i]];
		}

		/* Allocate memory used for decoded main stream bytes. */
		if (zip->tmp_stream_buff == NULL) {
			zip->tmp_stream_buff_size = 32 * 1024;
			zip->tmp_stream_buff =
			    malloc(zip->tmp_stream_buff_size);
			if (zip->tmp_stream_buff == NULL) {
				archive_set_error(&a->archive, ENOMEM,
				    "No memory for 7-Zip decompression");
				return (ARCHIVE_FATAL);
			}
		}
		zip->tmp_stream_bytes_avail = 0;
		zip->tmp_stream_bytes_remaining = 0;
		zip->odd_bcj_size = 0;
		zip->bcj2_outPos = 0;

		/*
		 * Reset a stream reader in order to read the main stream
		 * of BCJ2.
		 */
		zip->pack_stream_remaining = 1;
		zip->pack_stream_index = (unsigned)folder->packIndex;
		zip->folder_outbytes_remaining =
		    folder_uncompressed_size(folder);
		zip->uncompressed_buffer_bytes_remaining = 0;
	}

	/*
	 * Initialize the decompressor for the new folder's pack streams.
	 */
	r = init_decompression(a, zip, coder1, coder2);
	if (r != ARCHIVE_OK)
		return (ARCHIVE_FATAL);
	return (ARCHIVE_OK);
}

static int64_t
skip_stream(struct archive_read *a, size_t skip_bytes)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	const void *p;
	int64_t skipped_bytes;
	size_t bytes = skip_bytes;

	if (zip->folder_index == 0) {
		/*
		 * Optimization for a list mode.
		 * Avoid unncecessary decoding operations.
		 */
		zip->si.ci.folders[zip->entry->folderIndex].skipped_bytes
		    += skip_bytes;
		return (skip_bytes);
	}

	while (bytes) {
		skipped_bytes = read_stream(a, &p, bytes);
		if (skipped_bytes < 0)
			return (skipped_bytes);
		if (skipped_bytes == 0) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated 7-Zip file body");
			return (ARCHIVE_FATAL);
		}
		bytes -= skipped_bytes;
		if (zip->pack_stream_bytes_unconsumed)
			read_consume(a);
	}
	return (skip_bytes);
}

/*
 * Brought from LZMA SDK.
 *
 * Bra86.c -- Converter for x86 code (BCJ)
 * 2008-10-04 : Igor Pavlov : Public domain
 *
 */

#define Test86MSByte(b) ((b) == 0 || (b) == 0xFF)

static const unsigned char kMaskToAllowedStatus[8] = {1, 1, 1, 0, 1, 0, 0, 0};
static const unsigned char kMaskToBitNumber[8] = {0, 1, 2, 2, 3, 3, 3, 3};

static size_t
x86_Convert(uint8_t *data, size_t size, uint32_t ip, uint32_t *state)
{
	size_t bufferPos = 0, prevPosT;
	uint32_t prevMask = *state & 0x7;
	if (size < 5)
		return 0;
	ip += 5;
	prevPosT = (size_t)0 - 1;

	for (;;) {
		uint8_t *p = data + bufferPos;
		uint8_t *limit = data + size - 4;

		for (; p < limit; p++)
			if ((*p & 0xFE) == 0xE8)
				break;
		bufferPos = (size_t)(p - data);
		if (p >= limit)
			break;
		prevPosT = bufferPos - prevPosT;
		if (prevPosT > 3)
			prevMask = 0;
		else {
			prevMask = (prevMask << ((int)prevPosT - 1)) & 0x7;
			if (prevMask != 0) {
				unsigned char b =
					p[4 - kMaskToBitNumber[prevMask]];
				if (!kMaskToAllowedStatus[prevMask] ||
				    Test86MSByte(b)) {
					prevPosT = bufferPos;
					prevMask = ((prevMask << 1) & 0x7) | 1;
					bufferPos++;
					continue;
				}
			}
		}
		prevPosT = bufferPos;

		if (Test86MSByte(p[4])) {
			uint32_t src = ((uint32_t)p[4] << 24) |
				((uint32_t)p[3] << 16) | ((uint32_t)p[2] << 8) |
				((uint32_t)p[1]);
			uint32_t dest;
			for (;;) {
				uint8_t b;
				int index;

				dest = src - (ip + (uint32_t)bufferPos);
				if (prevMask == 0)
					break;
				index = kMaskToBitNumber[prevMask] * 8;
				b = (uint8_t)(dest >> (24 - index));
				if (!Test86MSByte(b))
					break;
				src = dest ^ ((1 << (32 - index)) - 1);
			}
			p[4] = (uint8_t)(~(((dest >> 24) & 1) - 1));
			p[3] = (uint8_t)(dest >> 16);
			p[2] = (uint8_t)(dest >> 8);
			p[1] = (uint8_t)dest;
			bufferPos += 5;
		} else {
			prevMask = ((prevMask << 1) & 0x7) | 1;
			bufferPos++;
		}
	}
	prevPosT = bufferPos - prevPosT;
	*state = ((prevPosT > 3) ? 
			0 : ((prevMask << ((int)prevPosT - 1)) & 0x7));
	return (bufferPos);
}

/*
 * Brought from LZMA SDK.
 *
 * Bcj2.c -- Converter for x86 code (BCJ2)
 * 2008-10-04 : Igor Pavlov : Public domain
 *
 */

#define SZ_ERROR_DATA	 ARCHIVE_FAILED

#define IsJcc(b0, b1) ((b0) == 0x0F && ((b1) & 0xF0) == 0x80)
#define IsJ(b0, b1) ((b1 & 0xFE) == 0xE8 || IsJcc(b0, b1))

#define kNumTopBits 24
#define kTopValue ((uint32_t)1 << kNumTopBits)

#define kNumBitModelTotalBits 11
#define kBitModelTotal (1 << kNumBitModelTotalBits)
#define kNumMoveBits 5

#define RC_READ_BYTE (*buffer++)
#define RC_TEST { if (buffer == bufferLim) return SZ_ERROR_DATA; }
#define RC_INIT2 zip->bcj2_code = 0; zip->bcj2_range = 0xFFFFFFFF; \
  { int i; for (i = 0; i < 5; i++) { RC_TEST; zip->bcj2_code = (zip->bcj2_code << 8) | RC_READ_BYTE; }}

#define NORMALIZE if (zip->bcj2_range < kTopValue) { RC_TEST; zip->bcj2_range <<= 8; zip->bcj2_code = (zip->bcj2_code << 8) | RC_READ_BYTE; }

#define IF_BIT_0(p) ttt = *(p); bound = (zip->bcj2_range >> kNumBitModelTotalBits) * ttt; if (zip->bcj2_code < bound)
#define UPDATE_0(p) zip->bcj2_range = bound; *(p) = (CProb)(ttt + ((kBitModelTotal - ttt) >> kNumMoveBits)); NORMALIZE;
#define UPDATE_1(p) zip->bcj2_range -= bound; zip->bcj2_code -= bound; *(p) = (CProb)(ttt - (ttt >> kNumMoveBits)); NORMALIZE;

ssize_t
Bcj2_Decode(struct _7zip *zip, uint8_t *outBuf, size_t outSize)
{
	size_t inPos = 0, outPos = 0;
	const uint8_t *buf0, *buf1, *buf2, *buf3;
	size_t size0, size1, size2, size3;
	const uint8_t *buffer, *bufferLim;
	unsigned int i, j;

	size0 = zip->tmp_stream_bytes_remaining;
	buf0 = zip->tmp_stream_buff + zip->tmp_stream_bytes_avail - size0;
	size1 = zip->sub_stream_bytes_remaining[0];
	buf1 = zip->sub_stream_buff[0] + zip->sub_stream_size[0] - size1;
	size2 = zip->sub_stream_bytes_remaining[1];
	buf2 = zip->sub_stream_buff[1] + zip->sub_stream_size[1] - size2;
	size3 = zip->sub_stream_bytes_remaining[2];
	buf3 = zip->sub_stream_buff[2] + zip->sub_stream_size[2] - size3;

	buffer = buf3;
	bufferLim = buffer + size3;

	if (zip->bcj_state == 0) {
		/*
		 * Initialize.
		 */
		zip->bcj2_prevByte = 0;
		for (i = 0;
		    i < sizeof(zip->bcj2_p) / sizeof(zip->bcj2_p[0]); i++)
			zip->bcj2_p[i] = kBitModelTotal >> 1;
		RC_INIT2;
		zip->bcj_state = 1;
	}

	/*
	 * Gather the odd bytes of a previous call.
	 */
	for (i = 0; zip->odd_bcj_size > 0 && outPos < outSize; i++) {
		outBuf[outPos++] = zip->odd_bcj[i];
		zip->odd_bcj_size--;
	}

	if (outSize == 0) {
		zip->bcj2_outPos += outPos;
		return (outPos);
	}

	for (;;) {
		uint8_t b;
		CProb *prob;
		uint32_t bound;
		uint32_t ttt;

		size_t limit = size0 - inPos;
		if (outSize - outPos < limit)
			limit = outSize - outPos;

		if (zip->bcj_state == 1) {
			while (limit != 0) {
				uint8_t b = buf0[inPos];
				outBuf[outPos++] = b;
				if (IsJ(zip->bcj2_prevByte, b)) {
					zip->bcj_state = 2;
					break;
				}
				inPos++;
				zip->bcj2_prevByte = b;
				limit--;
			}
		}

		if (limit == 0 || outPos == outSize)
			break;
		zip->bcj_state = 1;

		b = buf0[inPos++];

		if (b == 0xE8)
			prob = zip->bcj2_p + zip->bcj2_prevByte;
		else if (b == 0xE9)
			prob = zip->bcj2_p + 256;
		else
			prob = zip->bcj2_p + 257;

		IF_BIT_0(prob) {
			UPDATE_0(prob)
			zip->bcj2_prevByte = b;
		} else {
			uint32_t dest;
			const uint8_t *v;
			uint8_t out[4];

			UPDATE_1(prob)
			if (b == 0xE8) {
				v = buf1;
				if (size1 < 4)
					return SZ_ERROR_DATA;
				buf1 += 4;
				size1 -= 4;
			} else {
				v = buf2;
				if (size2 < 4)
					return SZ_ERROR_DATA;
				buf2 += 4;
				size2 -= 4;
			}
			dest = (((uint32_t)v[0] << 24) |
			    ((uint32_t)v[1] << 16) |
			    ((uint32_t)v[2] << 8) |
			    ((uint32_t)v[3])) -
			    ((uint32_t)zip->bcj2_outPos + outPos + 4);
			out[0] = (uint8_t)dest;
			out[1] = (uint8_t)(dest >> 8);
			out[2] = (uint8_t)(dest >> 16);
			out[3] = zip->bcj2_prevByte = (uint8_t)(dest >> 24);

			for (i = 0; i < 4 && outPos < outSize; i++)
				outBuf[outPos++] = out[i];
			if (i < 4) {
				/*
				 * Save odd bytes which we could not add into
				 * the output buffer because of out of space.
				 */
				zip->odd_bcj_size = 4 -i;
				for (; i < 4; i++) {
					j = i - 4 + zip->odd_bcj_size;
					zip->odd_bcj[j] = out[i];
				}
				break;
			}
		}
	}
	zip->tmp_stream_bytes_remaining -= inPos;
	zip->sub_stream_bytes_remaining[0] = size1;
	zip->sub_stream_bytes_remaining[1] = size2;
	zip->sub_stream_bytes_remaining[2] = bufferLim - buffer;
	zip->bcj2_outPos += outPos;

	return ((ssize_t)outPos);
}

