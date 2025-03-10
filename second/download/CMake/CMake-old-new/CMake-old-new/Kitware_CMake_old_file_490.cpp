/*-
 * Copyright (c) 2003-2007 Tim Kientzle
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
__FBSDID("$FreeBSD: src/lib/libarchive/archive_write_set_format_pax.c,v 1.49 2008/09/30 03:57:07 kientzle Exp $");

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "archive.h"
#include "archive_entry.h"
#include "archive_private.h"
#include "archive_write_private.h"

struct pax {
    uint64_t    entry_bytes_remaining;
    uint64_t    entry_padding;
    struct archive_string   pax_header;
};

static void      add_pax_attr(struct archive_string *, const char *key,
                 const char *value);
static void      add_pax_attr_int(struct archive_string *,
                 const char *key, int64_t value);
static void      add_pax_attr_time(struct archive_string *,
                 const char *key, int64_t sec,
                 unsigned long nanos);
static void      add_pax_attr_w(struct archive_string *,
                 const char *key, const wchar_t *wvalue);
static ssize_t       archive_write_pax_data(struct archive_write *,
                 const void *, size_t);
static int       archive_write_pax_finish(struct archive_write *);
static int       archive_write_pax_destroy(struct archive_write *);
static int       archive_write_pax_finish_entry(struct archive_write *);
static int       archive_write_pax_header(struct archive_write *,
                 struct archive_entry *);
static char     *base64_encode(const char *src, size_t len);
static char     *build_pax_attribute_name(char *dest, const char *src);
static char     *build_ustar_entry_name(char *dest, const char *src,
                 size_t src_length, const char *insert);
static char     *format_int(char *dest, int64_t);
static int       has_non_ASCII(const wchar_t *);
static char     *url_encode(const char *in);
static int       write_nulls(struct archive_write *, size_t);

/*
 * Set output format to 'restricted pax' format.
 *
 * This is the same as normal 'pax', but tries to suppress
 * the pax header whenever possible.  This is the default for
 * bsdtar, for instance.
 */
int
archive_write_set_format_pax_restricted(struct archive *_a)
{
    struct archive_write *a = (struct archive_write *)_a;
    int r;
    r = archive_write_set_format_pax(&a->archive);
    a->archive.archive_format = ARCHIVE_FORMAT_TAR_PAX_RESTRICTED;
    a->archive.archive_format_name = "restricted POSIX pax interchange";
    return (r);
}

/*
 * Set output format to 'pax' format.
 */
int
archive_write_set_format_pax(struct archive *_a)
{
    struct archive_write *a = (struct archive_write *)_a;
    struct pax *pax;

    if (a->format_destroy != NULL)
        (a->format_destroy)(a);

    pax = (struct pax *)malloc(sizeof(*pax));
    if (pax == NULL) {
        archive_set_error(&a->archive, ENOMEM, "Can't allocate pax data");
        return (ARCHIVE_FATAL);
    }
    memset(pax, 0, sizeof(*pax));
    a->format_data = pax;

    a->pad_uncompressed = 1;
    a->format_name = "pax";
    a->format_write_header = archive_write_pax_header;
    a->format_write_data = archive_write_pax_data;
    a->format_finish = archive_write_pax_finish;
    a->format_destroy = archive_write_pax_destroy;
    a->format_finish_entry = archive_write_pax_finish_entry;
    a->archive.archive_format = ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE;
    a->archive.archive_format_name = "POSIX pax interchange";
    return (ARCHIVE_OK);
}

/*
 * Note: This code assumes that 'nanos' has the same sign as 'sec',
 * which implies that sec=-1, nanos=200000000 represents -1.2 seconds
 * and not -0.8 seconds.  This is a pretty pedantic point, as we're
 * unlikely to encounter many real files created before Jan 1, 1970,
 * much less ones with timestamps recorded to sub-second resolution.
 */
static void
add_pax_attr_time(struct archive_string *as, const char *key,
    int64_t sec, unsigned long nanos)
{
    int digit, i;
    char *t;
    /*
     * Note that each byte contributes fewer than 3 base-10
     * digits, so this will always be big enough.
     */
    char tmp[1 + 3*sizeof(sec) + 1 + 3*sizeof(nanos)];

    tmp[sizeof(tmp) - 1] = 0;
    t = tmp + sizeof(tmp) - 1;

    /* Skip trailing zeros in the fractional part. */
    for (digit = 0, i = 10; i > 0 && digit == 0; i--) {
        digit = nanos % 10;
        nanos /= 10;
    }

    /* Only format the fraction if it's non-zero. */
    if (i > 0) {
        while (i > 0) {
            *--t = "0123456789"[digit];
            digit = nanos % 10;
            nanos /= 10;
            i--;
        }
        *--t = '.';
    }
    t = format_int(t, sec);

    add_pax_attr(as, key, t);
}

static char *
format_int(char *t, int64_t i)
{
    int sign;

    if (i < 0) {
        sign = -1;
        i = -i;
    } else
        sign = 1;

    do {
        *--t = "0123456789"[i % 10];
    } while (i /= 10);
    if (sign < 0)
        *--t = '-';
    return (t);
}

static void
add_pax_attr_int(struct archive_string *as, const char *key, int64_t value)
{
    char tmp[1 + 3 * sizeof(value)];

    tmp[sizeof(tmp) - 1] = 0;
    add_pax_attr(as, key, format_int(tmp + sizeof(tmp) - 1, value));
}

static char *
utf8_encode(const wchar_t *wval)
{
    int utf8len;
    const wchar_t *wp;
    unsigned long wc;
    char *utf8_value, *p;

    utf8len = 0;
    for (wp = wval; *wp != L'\0'; ) {
        wc = *wp++;

        if (wc >= 0xd800 && wc <= 0xdbff
            && *wp >= 0xdc00 && *wp <= 0xdfff) {
            /* This is a surrogate pair.  Combine into a
             * full Unicode value before encoding into
             * UTF-8. */
            wc = (wc - 0xd800) << 10; /* High 10 bits */
            wc += (*wp++ - 0xdc00); /* Low 10 bits */
            wc += 0x10000; /* Skip BMP */
        }
        if (wc <= 0x7f)
            utf8len++;
        else if (wc <= 0x7ff)
            utf8len += 2;
        else if (wc <= 0xffff)
            utf8len += 3;
        else if (wc <= 0x1fffff)
            utf8len += 4;
        else if (wc <= 0x3ffffff)
            utf8len += 5;
        else if (wc <= 0x7fffffff)
            utf8len += 6;
        /* Ignore larger values; UTF-8 can't encode them. */
    }

    utf8_value = (char *)malloc(utf8len + 1);
    if (utf8_value == NULL) {
        __archive_errx(1, "Not enough memory for attributes");
        return (NULL);
    }

    for (wp = wval, p = utf8_value; *wp != L'\0'; ) {
        wc = *wp++;
        if (wc >= 0xd800 && wc <= 0xdbff
            && *wp >= 0xdc00 && *wp <= 0xdfff) {
            /* Combine surrogate pair. */
            wc = (wc - 0xd800) << 10;
            wc += *wp++ - 0xdc00 + 0x10000;
        }
        if (wc <= 0x7f) {
            *p++ = (char)wc;
        } else if (wc <= 0x7ff) {
            p[0] = 0xc0 | ((wc >> 6) & 0x1f);
            p[1] = 0x80 | (wc & 0x3f);
            p += 2;
        } else if (wc <= 0xffff) {
            p[0] = 0xe0 | ((wc >> 12) & 0x0f);
            p[1] = 0x80 | ((wc >> 6) & 0x3f);
            p[2] = 0x80 | (wc & 0x3f);
            p += 3;
        } else if (wc <= 0x1fffff) {
            p[0] = 0xf0 | ((wc >> 18) & 0x07);
            p[1] = 0x80 | ((wc >> 12) & 0x3f);
            p[2] = 0x80 | ((wc >> 6) & 0x3f);
            p[3] = 0x80 | (wc & 0x3f);
            p += 4;
        } else if (wc <= 0x3ffffff) {
            p[0] = 0xf8 | ((wc >> 24) & 0x03);
            p[1] = 0x80 | ((wc >> 18) & 0x3f);
            p[2] = 0x80 | ((wc >> 12) & 0x3f);
            p[3] = 0x80 | ((wc >> 6) & 0x3f);
            p[4] = 0x80 | (wc & 0x3f);
            p += 5;
        } else if (wc <= 0x7fffffff) {
            p[0] = 0xfc | ((wc >> 30) & 0x01);
            p[1] = 0x80 | ((wc >> 24) & 0x3f);
            p[1] = 0x80 | ((wc >> 18) & 0x3f);
            p[2] = 0x80 | ((wc >> 12) & 0x3f);
            p[3] = 0x80 | ((wc >> 6) & 0x3f);
            p[4] = 0x80 | (wc & 0x3f);
            p += 6;
        }
        /* Ignore larger values; UTF-8 can't encode them. */
    }
    *p = '\0';

    return (utf8_value);
}

static void
add_pax_attr_w(struct archive_string *as, const char *key, const wchar_t *wval)
{
    char *utf8_value = utf8_encode(wval);
    if (utf8_value == NULL)
        return;
    add_pax_attr(as, key, utf8_value);
    free(utf8_value);
}

/*
 * Add a key/value attribute to the pax header.  This function handles
 * the length field and various other syntactic requirements.
 */
static void
add_pax_attr(struct archive_string *as, const char *key, const char *value)
{
    int digits, i, len, next_ten;
    char tmp[1 + 3 * sizeof(int)];  /* < 3 base-10 digits per byte */

    /*-
     * PAX attributes have the following layout:
     *     <len> <space> <key> <=> <value> <nl>
     */
    len = 1 + strlen(key) + 1 + strlen(value) + 1;

    /*
     * The <len> field includes the length of the <len> field, so
     * computing the correct length is tricky.  I start by
     * counting the number of base-10 digits in 'len' and
     * computing the next higher power of 10.
     */
    next_ten = 1;
    digits = 0;
    i = len;
    while (i > 0) {
        i = i / 10;
        digits++;
        next_ten = next_ten * 10;
    }
    /*
     * For example, if string without the length field is 99
     * chars, then adding the 2 digit length "99" will force the
     * total length past 100, requiring an extra digit.  The next
     * statement adjusts for this effect.
     */
    if (len + digits >= next_ten)
        digits++;

    /* Now, we have the right length so we can build the line. */
    tmp[sizeof(tmp) - 1] = 0;   /* Null-terminate the work area. */
    archive_strcat(as, format_int(tmp + sizeof(tmp) - 1, len + digits));
    archive_strappend_char(as, ' ');
    archive_strcat(as, key);
    archive_strappend_char(as, '=');
    archive_strcat(as, value);
    archive_strappend_char(as, '\n');
}

static void
archive_write_pax_header_xattrs(struct pax *pax, struct archive_entry *entry)
{
    struct archive_string s;
    int i = archive_entry_xattr_reset(entry);

    while (i--) {
        const char *name;
        const void *value;
        char *encoded_value;
        char *url_encoded_name = NULL, *encoded_name = NULL;
        wchar_t *wcs_name = NULL;
        size_t size;

        archive_entry_xattr_next(entry, &name, &value, &size);
        /* Name is URL-encoded, then converted to wchar_t,
         * then UTF-8 encoded. */
        url_encoded_name = url_encode(name);
        if (url_encoded_name != NULL) {
            /* Convert narrow-character to wide-character. */
            int wcs_length = strlen(url_encoded_name);
            wcs_name = (wchar_t *)malloc((wcs_length + 1) * sizeof(wchar_t));
            if (wcs_name == NULL)
                __archive_errx(1, "No memory for xattr conversion");
            mbstowcs(wcs_name, url_encoded_name, wcs_length);
            wcs_name[wcs_length] = 0;
            free(url_encoded_name); /* Done with this. */
        }
        if (wcs_name != NULL) {
            encoded_name = utf8_encode(wcs_name);
            free(wcs_name); /* Done with wchar_t name. */
        }

        encoded_value = base64_encode((const char *)value, size);

        if (encoded_name != NULL && encoded_value != NULL) {
            archive_string_init(&s);
            archive_strcpy(&s, "LIBARCHIVE.xattr.");
            archive_strcat(&s, encoded_name);
            add_pax_attr(&(pax->pax_header), s.s, encoded_value);
            archive_string_free(&s);
        }
        free(encoded_name);
        free(encoded_value);
    }
}

/*
 * TODO: Consider adding 'comment' and 'charset' fields to
 * archive_entry so that clients can specify them.  Also, consider
 * adding generic key/value tags so clients can add arbitrary
 * key/value data.
 */
static int
archive_write_pax_header(struct archive_write *a,
    struct archive_entry *entry_original)
{
    struct archive_entry *entry_main;
    const char *p;
    char *t;
    const wchar_t *wp;
    const char *suffix;
    int need_extension, r, ret;
    struct pax *pax;
    const char *hdrcharset = NULL;
    const char *hardlink;
    const char *path = NULL, *linkpath = NULL;
    const char *uname = NULL, *gname = NULL;
    const wchar_t *path_w = NULL, *linkpath_w = NULL;
    const wchar_t *uname_w = NULL, *gname_w = NULL;

    char paxbuff[512];
    char ustarbuff[512];
    char ustar_entry_name[256];
    char pax_entry_name[256];

    ret = ARCHIVE_OK;
    need_extension = 0;
    pax = (struct pax *)a->format_data;

    hardlink = archive_entry_hardlink(entry_original);

    /* Make sure this is a type of entry that we can handle here */
    if (hardlink == NULL) {
        switch (archive_entry_filetype(entry_original)) {
        case AE_IFBLK:
        case AE_IFCHR:
        case AE_IFIFO:
        case AE_IFLNK:
        case AE_IFREG:
            break;
        case AE_IFDIR:
            /*
             * Ensure a trailing '/'.  Modify the original
             * entry so the client sees the change.
             */
            p = archive_entry_pathname(entry_original);
            if (p[strlen(p) - 1] != '/') {
                t = (char *)malloc(strlen(p) + 2);
                if (t == NULL) {
                    archive_set_error(&a->archive, ENOMEM,
                    "Can't allocate pax data");
                    return(ARCHIVE_FATAL);
                }
                strcpy(t, p);
                strcat(t, "/");
                archive_entry_copy_pathname(entry_original, t);
                free(t);
            }
            break;
        case AE_IFSOCK:
            archive_set_error(&a->archive,
                ARCHIVE_ERRNO_FILE_FORMAT,
                "tar format cannot archive socket");
            return (ARCHIVE_WARN);
        default:
            archive_set_error(&a->archive,
                ARCHIVE_ERRNO_FILE_FORMAT,
                "tar format cannot archive this (type=0%lo)",
                (unsigned long)archive_entry_filetype(entry_original));
            return (ARCHIVE_WARN);
        }
    }

    /* Copy entry so we can modify it as needed. */
    entry_main = archive_entry_clone(entry_original);
    archive_string_empty(&(pax->pax_header)); /* Blank our work area. */

    /*
     * First, check the name fields and see if any of them
     * require binary coding.  If any of them does, then all of
     * them do.
     */
    hdrcharset = NULL;
    path = archive_entry_pathname(entry_main);
    path_w = archive_entry_pathname_w(entry_main);
    if (path != NULL && path_w == NULL) {
        archive_set_error(&a->archive, EILSEQ,
            "Can't translate pathname '%s' to UTF-8", path);
        ret = ARCHIVE_WARN;
        hdrcharset = "BINARY";
    }
    uname = archive_entry_uname(entry_main);
    uname_w = archive_entry_uname_w(entry_main);
    if (uname != NULL && uname_w == NULL) {
        archive_set_error(&a->archive, EILSEQ,
            "Can't translate uname '%s' to UTF-8", uname);
        ret = ARCHIVE_WARN;
        hdrcharset = "BINARY";
    }
    gname = archive_entry_gname(entry_main);
    gname_w = archive_entry_gname_w(entry_main);
    if (gname != NULL && gname_w == NULL) {
        archive_set_error(&a->archive, EILSEQ,
            "Can't translate gname '%s' to UTF-8", gname);
        ret = ARCHIVE_WARN;
        hdrcharset = "BINARY";
    }
    linkpath = hardlink;
    if (linkpath != NULL) {
        linkpath_w = archive_entry_hardlink_w(entry_main);
    } else {
        linkpath = archive_entry_symlink(entry_main);
        if (linkpath != NULL)
            linkpath_w = archive_entry_symlink_w(entry_main);
    }
    if (linkpath != NULL && linkpath_w == NULL) {
        archive_set_error(&a->archive, EILSEQ,
            "Can't translate linkpath '%s' to UTF-8", linkpath);
        ret = ARCHIVE_WARN;
        hdrcharset = "BINARY";
    }

    /* Store the header encoding first, to be nice to readers. */
    if (hdrcharset != NULL)
        add_pax_attr(&(pax->pax_header), "hdrcharset", hdrcharset);


    /*
     * If name is too long, or has non-ASCII characters, add
     * 'path' to pax extended attrs.  (Note that an unconvertible
     * name must have non-ASCII characters.)
     */
    if (path == NULL) {
        /* We don't have a narrow version, so we have to store
         * the wide version. */
        add_pax_attr_w(&(pax->pax_header), "path", path_w);
        archive_entry_set_pathname(entry_main, "@WidePath");
        need_extension = 1;
    } else if (has_non_ASCII(path_w)) {
        /* We have non-ASCII characters. */
        if (path_w == NULL || hdrcharset != NULL) {
            /* Can't do UTF-8, so store it raw. */
            add_pax_attr(&(pax->pax_header), "path", path);
        } else {
            /* Store UTF-8 */
            add_pax_attr_w(&(pax->pax_header),
                "path", path_w);
        }
        archive_entry_set_pathname(entry_main,
            build_ustar_entry_name(ustar_entry_name,
            path, strlen(path), NULL));
        need_extension = 1;
    } else {
        /* We have an all-ASCII path; we'd like to just store
         * it in the ustar header if it will fit.  Yes, this
         * duplicates some of the logic in
         * write_set_format_ustar.c
         */
        if (strlen(path) <= 100) {
            /* Fits in the old 100-char tar name field. */
        } else {
            /* Find largest suffix that will fit. */
            /* Note: strlen() > 100, so strlen() - 100 - 1 >= 0 */
            suffix = strchr(path + strlen(path) - 100 - 1, '/');
            /* Don't attempt an empty prefix. */
            if (suffix == path)
                suffix = strchr(suffix + 1, '/');
            /* We can put it in the ustar header if it's
             * all ASCII and it's either <= 100 characters
             * or can be split at a '/' into a prefix <=
             * 155 chars and a suffix <= 100 chars.  (Note
             * the strchr() above will return NULL exactly
             * when the path can't be split.)
             */
            if (suffix == NULL       /* Suffix > 100 chars. */
                || suffix[1] == '\0'    /* empty suffix */
                || suffix - path > 155)  /* Prefix > 155 chars */
            {
                if (path_w == NULL || hdrcharset != NULL) {
                    /* Can't do UTF-8, so store it raw. */
                    add_pax_attr(&(pax->pax_header),
                        "path", path);
                } else {
                    /* Store UTF-8 */
                    add_pax_attr_w(&(pax->pax_header),
                        "path", path_w);
                }
                archive_entry_set_pathname(entry_main,
                    build_ustar_entry_name(ustar_entry_name,
                    path, strlen(path), NULL));
                need_extension = 1;
            }
        }
    }

    if (linkpath != NULL) {
        /* If link name is too long or has non-ASCII characters, add
         * 'linkpath' to pax extended attrs. */
        if (strlen(linkpath) > 100 || linkpath_w == NULL
            || linkpath_w == NULL || has_non_ASCII(linkpath_w)) {
            if (linkpath_w == NULL || hdrcharset != NULL)
                /* If the linkpath is not convertible
                 * to wide, or we're encoding in
                 * binary anyway, store it raw. */
                add_pax_attr(&(pax->pax_header),
                    "linkpath", linkpath);
            else
                /* If the link is long or has a
                 * non-ASCII character, store it as a
                 * pax extended attribute. */
                add_pax_attr_w(&(pax->pax_header),
                    "linkpath", linkpath_w);
            if (strlen(linkpath) > 100) {
                if (hardlink != NULL)
                    archive_entry_set_hardlink(entry_main,
                        "././@LongHardLink");
                else
                    archive_entry_set_symlink(entry_main,
                        "././@LongSymLink");
            }
            need_extension = 1;
        }
    }

    /* If file size is too large, add 'size' to pax extended attrs. */
    if (archive_entry_size(entry_main) >= (((int64_t)1) << 33)) {
        add_pax_attr_int(&(pax->pax_header), "size",
            archive_entry_size(entry_main));
        need_extension = 1;
    }

    /* If numeric GID is too large, add 'gid' to pax extended attrs. */
    if ((unsigned int)archive_entry_gid(entry_main) >= (1 << 18)) {
        add_pax_attr_int(&(pax->pax_header), "gid",
            archive_entry_gid(entry_main));
        need_extension = 1;
    }

    /* If group name is too large or has non-ASCII characters, add
     * 'gname' to pax extended attrs. */
    if (gname != NULL) {
        if (strlen(gname) > 31
            || gname_w == NULL
            || has_non_ASCII(gname_w))
        {
            if (gname_w == NULL || hdrcharset != NULL) {
                add_pax_attr(&(pax->pax_header),
                    "gname", gname);
            } else  {
                add_pax_attr_w(&(pax->pax_header),
                    "gname", gname_w);
            }
            need_extension = 1;
        }
    }

    /* If numeric UID is too large, add 'uid' to pax extended attrs. */
    if ((unsigned int)archive_entry_uid(entry_main) >= (1 << 18)) {
        add_pax_attr_int(&(pax->pax_header), "uid",
            archive_entry_uid(entry_main));
        need_extension = 1;
    }

    /* Add 'uname' to pax extended attrs if necessary. */
    if (uname != NULL) {
        if (strlen(uname) > 31
            || uname_w == NULL
            || has_non_ASCII(uname_w))
        {
            if (uname_w == NULL || hdrcharset != NULL) {
                add_pax_attr(&(pax->pax_header),
                    "uname", uname);
            } else {
                add_pax_attr_w(&(pax->pax_header),
                    "uname", uname_w);
            }
            need_extension = 1;
        }
    }

    /*
     * POSIX/SUSv3 doesn't provide a standard key for large device
     * numbers.  I use the same keys here that Joerg Schilling
     * used for 'star.'  (Which, somewhat confusingly, are called
     * "devXXX" even though they code "rdev" values.)  No doubt,
     * other implementations use other keys.  Note that there's no
     * reason we can't write the same information into a number of
     * different keys.
     *
     * Of course, this is only needed for block or char device entries.
     */
    if (archive_entry_filetype(entry_main) == AE_IFBLK
        || archive_entry_filetype(entry_main) == AE_IFCHR) {
        /*
         * If rdevmajor is too large, add 'SCHILY.devmajor' to
         * extended attributes.
         */
        dev_t rdevmajor, rdevminor;
        rdevmajor = archive_entry_rdevmajor(entry_main);
        rdevminor = archive_entry_rdevminor(entry_main);
        if (rdevmajor >= (1 << 18)) {
            add_pax_attr_int(&(pax->pax_header), "SCHILY.devmajor",
                rdevmajor);
            /*
             * Non-strict formatting below means we don't
             * have to truncate here.  Not truncating improves
             * the chance that some more modern tar archivers
             * (such as GNU tar 1.13) can restore the full
             * value even if they don't understand the pax
             * extended attributes.  See my rant below about
             * file size fields for additional details.
             */
            /* archive_entry_set_rdevmajor(entry_main,
               rdevmajor & ((1 << 18) - 1)); */
            need_extension = 1;
        }

        /*
         * If devminor is too large, add 'SCHILY.devminor' to
         * extended attributes.
         */
        if (rdevminor >= (1 << 18)) {
            add_pax_attr_int(&(pax->pax_header), "SCHILY.devminor",
                rdevminor);
            /* Truncation is not necessary here, either. */
            /* archive_entry_set_rdevminor(entry_main,
               rdevminor & ((1 << 18) - 1)); */
            need_extension = 1;
        }
    }

    /*
     * Technically, the mtime field in the ustar header can
     * support 33 bits, but many platforms use signed 32-bit time
     * values.  The cutoff of 0x7fffffff here is a compromise.
     * Yes, this check is duplicated just below; this helps to
     * avoid writing an mtime attribute just to handle a
     * high-resolution timestamp in "restricted pax" mode.
     */
    if (!need_extension &&
        ((archive_entry_mtime(entry_main) < 0)
        || (archive_entry_mtime(entry_main) >= 0x7fffffff)))
        need_extension = 1;

    /* I use a star-compatible file flag attribute. */
    p = archive_entry_fflags_text(entry_main);
    if (!need_extension && p != NULL  &&  *p != '\0')
        need_extension = 1;

    /* If there are non-trivial ACL entries, we need an extension. */
    if (!need_extension && archive_entry_acl_count(entry_original,
        ARCHIVE_ENTRY_ACL_TYPE_ACCESS) > 0)
        need_extension = 1;

    /* If there are non-trivial ACL entries, we need an extension. */
    if (!need_extension && archive_entry_acl_count(entry_original,
        ARCHIVE_ENTRY_ACL_TYPE_DEFAULT) > 0)
        need_extension = 1;

    /* If there are extended attributes, we need an extension */
    if (!need_extension && archive_entry_xattr_count(entry_original) > 0)
        need_extension = 1;

    /*
     * The following items are handled differently in "pax
     * restricted" format.  In particular, in "pax restricted"
     * format they won't be added unless need_extension is
     * already set (we're already generating an extended header, so
     * may as well include these).
     */
    if (a->archive.archive_format != ARCHIVE_FORMAT_TAR_PAX_RESTRICTED ||
        need_extension) {

        if (archive_entry_mtime(entry_main) < 0  ||
            archive_entry_mtime(entry_main) >= 0x7fffffff  ||
            archive_entry_mtime_nsec(entry_main) != 0)
            add_pax_attr_time(&(pax->pax_header), "mtime",
                archive_entry_mtime(entry_main),
                archive_entry_mtime_nsec(entry_main));

        if (archive_entry_ctime(entry_main) != 0  ||
            archive_entry_ctime_nsec(entry_main) != 0)
            add_pax_attr_time(&(pax->pax_header), "ctime",
                archive_entry_ctime(entry_main),
                archive_entry_ctime_nsec(entry_main));

        if (archive_entry_atime(entry_main) != 0 ||
            archive_entry_atime_nsec(entry_main) != 0)
            add_pax_attr_time(&(pax->pax_header), "atime",
                archive_entry_atime(entry_main),
                archive_entry_atime_nsec(entry_main));

        /* Store birth/creationtime only if it's earlier than mtime */
        if (archive_entry_birthtime_is_set(entry_main) &&
            archive_entry_birthtime(entry_main)
            < archive_entry_mtime(entry_main))
            add_pax_attr_time(&(pax->pax_header),
                "LIBARCHIVE.creationtime",
                archive_entry_birthtime(entry_main),
                archive_entry_birthtime_nsec(entry_main));

        /* I use a star-compatible file flag attribute. */
        p = archive_entry_fflags_text(entry_main);
        if (p != NULL  &&  *p != '\0')
            add_pax_attr(&(pax->pax_header), "SCHILY.fflags", p);

        /* I use star-compatible ACL attributes. */
        wp = archive_entry_acl_text_w(entry_original,
            ARCHIVE_ENTRY_ACL_TYPE_ACCESS |
            ARCHIVE_ENTRY_ACL_STYLE_EXTRA_ID);
        if (wp != NULL && *wp != L'\0')
            add_pax_attr_w(&(pax->pax_header),
                "SCHILY.acl.access", wp);
        wp = archive_entry_acl_text_w(entry_original,
            ARCHIVE_ENTRY_ACL_TYPE_DEFAULT |
            ARCHIVE_ENTRY_ACL_STYLE_EXTRA_ID);
        if (wp != NULL && *wp != L'\0')
            add_pax_attr_w(&(pax->pax_header),
                "SCHILY.acl.default", wp);

        /* Include star-compatible metadata info. */
        /* Note: "SCHILY.dev{major,minor}" are NOT the
         * major/minor portions of "SCHILY.dev". */
        add_pax_attr_int(&(pax->pax_header), "SCHILY.dev",
            archive_entry_dev(entry_main));
        add_pax_attr_int(&(pax->pax_header), "SCHILY.ino",
            archive_entry_ino64(entry_main));
        add_pax_attr_int(&(pax->pax_header), "SCHILY.nlink",
            archive_entry_nlink(entry_main));

        /* Store extended attributes */
        archive_write_pax_header_xattrs(pax, entry_original);
    }

    /* Only regular files have data. */
    if (archive_entry_filetype(entry_main) != AE_IFREG)
        archive_entry_set_size(entry_main, 0);

    /*
     * Pax-restricted does not store data for hardlinks, in order
     * to improve compatibility with ustar.
     */
    if (a->archive.archive_format != ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE &&
        hardlink != NULL)
        archive_entry_set_size(entry_main, 0);

    /*
     * XXX Full pax interchange format does permit a hardlink
     * entry to have data associated with it.  I'm not supporting
     * that here because the client expects me to tell them whether
     * or not this format expects data for hardlinks.  If I
     * don't check here, then every pax archive will end up with
     * duplicated data for hardlinks.  Someday, there may be
     * need to select this behavior, in which case the following
     * will need to be revisited. XXX
     */
    if (hardlink != NULL)
        archive_entry_set_size(entry_main, 0);

    /* Format 'ustar' header for main entry.
     *
     * The trouble with file size: If the reader can't understand
     * the file size, they may not be able to locate the next
     * entry and the rest of the archive is toast.  Pax-compliant
     * readers are supposed to ignore the file size in the main
     * header, so the question becomes how to maximize portability
     * for readers that don't support pax attribute extensions.
     * For maximum compatibility, I permit numeric extensions in
     * the main header so that the file size stored will always be
     * correct, even if it's in a format that only some
     * implementations understand.  The technique used here is:
     *
     *  a) If possible, follow the standard exactly.  This handles
     *  files up to 8 gigabytes minus 1.
     *
     *  b) If that fails, try octal but omit the field terminator.
     *  That handles files up to 64 gigabytes minus 1.
     *
     *  c) Otherwise, use base-256 extensions.  That handles files
     *  up to 2^63 in this implementation, with the potential to
     *  go up to 2^94.  That should hold us for a while. ;-)
     *
     * The non-strict formatter uses similar logic for other
     * numeric fields, though they're less critical.
     */
    __archive_write_format_header_ustar(a, ustarbuff, entry_main, -1, 0);

    /* If we built any extended attributes, write that entry first. */
    if (archive_strlen(&(pax->pax_header)) > 0) {
        struct archive_entry *pax_attr_entry;
        time_t s;
        uid_t uid;
        gid_t gid;
        mode_t mode;
        long ns;

        pax_attr_entry = archive_entry_new();
        p = archive_entry_pathname(entry_main);
        archive_entry_set_pathname(pax_attr_entry,
            build_pax_attribute_name(pax_entry_name, p));
        archive_entry_set_size(pax_attr_entry,
            archive_strlen(&(pax->pax_header)));
        /* Copy uid/gid (but clip to ustar limits). */
        uid = archive_entry_uid(entry_main);
        if ((unsigned int)uid >= 1 << 18)
            uid = (uid_t)(1 << 18) - 1;
        archive_entry_set_uid(pax_attr_entry, uid);
        gid = archive_entry_gid(entry_main);
        if ((unsigned int)gid >= 1 << 18)
            gid = (gid_t)(1 << 18) - 1;
        archive_entry_set_gid(pax_attr_entry, gid);
        /* Copy mode over (but not setuid/setgid bits) */
        mode = archive_entry_mode(entry_main);
#ifdef S_ISUID
        mode &= ~S_ISUID;
#endif
#ifdef S_ISGID
        mode &= ~S_ISGID;
#endif
#ifdef S_ISVTX
        mode &= ~S_ISVTX;
#endif
        archive_entry_set_mode(pax_attr_entry, mode);

        /* Copy uname/gname. */
        archive_entry_set_uname(pax_attr_entry,
            archive_entry_uname(entry_main));
        archive_entry_set_gname(pax_attr_entry,
            archive_entry_gname(entry_main));

        /* Copy mtime, but clip to ustar limits. */
        s = archive_entry_mtime(entry_main);
        ns = archive_entry_mtime_nsec(entry_main);
        if (s < 0) { s = 0; ns = 0; }
        if (s > 0x7fffffff) { s = 0x7fffffff; ns = 0; }
        archive_entry_set_mtime(pax_attr_entry, s, ns);

        /* Ditto for atime. */
        s = archive_entry_atime(entry_main);
        ns = archive_entry_atime_nsec(entry_main);
        if (s < 0) { s = 0; ns = 0; }
        if (s > 0x7fffffff) { s = 0x7fffffff; ns = 0; }
        archive_entry_set_atime(pax_attr_entry, s, ns);

        /* Standard ustar doesn't support ctime. */
        archive_entry_set_ctime(pax_attr_entry, 0, 0);

        r = __archive_write_format_header_ustar(a, paxbuff,
            pax_attr_entry, 'x', 1);

        archive_entry_free(pax_attr_entry);

        /* Note that the 'x' header shouldn't ever fail to format */
        if (r != 0) {
            const char *msg = "archive_write_pax_header: "
                "'x' header failed?!  This can't happen.\n";
            size_t u = write(2, msg, strlen(msg));
            (void)u; /* UNUSED */
            exit(1);
        }
        r = (a->compressor.write)(a, paxbuff, 512);
        if (r != ARCHIVE_OK) {
            pax->entry_bytes_remaining = 0;
            pax->entry_padding = 0;
            return (ARCHIVE_FATAL);
        }

        pax->entry_bytes_remaining = archive_strlen(&(pax->pax_header));
        pax->entry_padding = 0x1ff & (-(int64_t)pax->entry_bytes_remaining);

        r = (a->compressor.write)(a, pax->pax_header.s,
            archive_strlen(&(pax->pax_header)));
        if (r != ARCHIVE_OK) {
            /* If a write fails, we're pretty much toast. */
            return (ARCHIVE_FATAL);
        }
        /* Pad out the end of the entry. */
        r = write_nulls(a, pax->entry_padding);
        if (r != ARCHIVE_OK) {
            /* If a write fails, we're pretty much toast. */
            return (ARCHIVE_FATAL);
        }
        pax->entry_bytes_remaining = pax->entry_padding = 0;
    }

    /* Write the header for main entry. */
    r = (a->compressor.write)(a, ustarbuff, 512);
    if (r != ARCHIVE_OK)
        return (r);

    /*
     * Inform the client of the on-disk size we're using, so
     * they can avoid unnecessarily writing a body for something
     * that we're just going to ignore.
     */
    archive_entry_set_size(entry_original, archive_entry_size(entry_main));
    pax->entry_bytes_remaining = archive_entry_size(entry_main);
    pax->entry_padding = 0x1ff & (-(int64_t)pax->entry_bytes_remaining);
    archive_entry_free(entry_main);

    return (ret);
}

/*
 * We need a valid name for the regular 'ustar' entry.  This routine
 * tries to hack something more-or-less reasonable.
 *
 * The approach here tries to preserve leading dir names.  We do so by
 * working with four sections:
 *   1) "prefix" directory names,
 *   2) "suffix" directory names,
 *   3) inserted dir name (optional),
 *   4) filename.
 *
 * These sections must satisfy the following requirements:
 *   * Parts 1 & 2 together form an initial portion of the dir name.
 *   * Part 3 is specified by the caller.  (It should not contain a leading
 *     or trailing '/'.)
 *   * Part 4 forms an initial portion of the base filename.
 *   * The filename must be <= 99 chars to fit the ustar 'name' field.
 *   * Parts 2, 3, 4 together must be <= 99 chars to fit the ustar 'name' fld.
 *   * Part 1 must be <= 155 chars to fit the ustar 'prefix' field.
 *   * If the original name ends in a '/', the new name must also end in a '/'
 *   * Trailing '/.' sequences may be stripped.
 *
 * Note: Recall that the ustar format does not store the '/' separating
 * parts 1 & 2, but does store the '/' separating parts 2 & 3.
 */
static char *
build_ustar_entry_name(char *dest, const char *src, size_t src_length,
    const char *insert)
{
    const char *prefix, *prefix_end;
    const char *suffix, *suffix_end;
    const char *filename, *filename_end;
    char *p;
    int need_slash = 0; /* Was there a trailing slash? */
    size_t suffix_length = 99;
    int insert_length;

    /* Length of additional dir element to be added. */
    if (insert == NULL)
        insert_length = 0;
    else
        /* +2 here allows for '/' before and after the insert. */
        insert_length = strlen(insert) + 2;

    /* Step 0: Quick bailout in a common case. */
    if (src_length < 100 && insert == NULL) {
        strncpy(dest, src, src_length);
        dest[src_length] = '\0';
        return (dest);
    }

    /* Step 1: Locate filename and enforce the length restriction. */
    filename_end = src + src_length;
    /* Remove trailing '/' chars and '/.' pairs. */
    for (;;) {
        if (filename_end > src && filename_end[-1] == '/') {
            filename_end --;
            need_slash = 1; /* Remember to restore trailing '/'. */
            continue;
        }
        if (filename_end > src + 1 && filename_end[-1] == '.'
            && filename_end[-2] == '/') {
            filename_end -= 2;
            need_slash = 1; /* "foo/." will become "foo/" */
            continue;
        }
        break;
    }
    if (need_slash)
        suffix_length--;
    /* Find start of filename. */
    filename = filename_end - 1;
    while ((filename > src) && (*filename != '/'))
        filename --;
    if ((*filename == '/') && (filename < filename_end - 1))
        filename ++;
    /* Adjust filename_end so that filename + insert fits in 99 chars. */
    suffix_length -= insert_length;
    if (filename_end > filename + suffix_length)
        filename_end = filename + suffix_length;
    /* Calculate max size for "suffix" section (#3 above). */
    suffix_length -= filename_end - filename;

    /* Step 2: Locate the "prefix" section of the dirname, including
     * trailing '/'. */
    prefix = src;
    prefix_end = prefix + 155;
    if (prefix_end > filename)
        prefix_end = filename;
    while (prefix_end > prefix && *prefix_end != '/')
        prefix_end--;
    if ((prefix_end < filename) && (*prefix_end == '/'))
        prefix_end++;

    /* Step 3: Locate the "suffix" section of the dirname,
     * including trailing '/'. */
    suffix = prefix_end;
    suffix_end = suffix + suffix_length; /* Enforce limit. */
    if (suffix_end > filename)
        suffix_end = filename;
    if (suffix_end < suffix)
        suffix_end = suffix;
    while (suffix_end > suffix && *suffix_end != '/')
        suffix_end--;
    if ((suffix_end < filename) && (*suffix_end == '/'))
        suffix_end++;

    /* Step 4: Build the new name. */
    /* The OpenBSD strlcpy function is safer, but less portable. */
    /* Rather than maintain two versions, just use the strncpy version. */
    p = dest;
    if (prefix_end > prefix) {
        strncpy(p, prefix, prefix_end - prefix);
        p += prefix_end - prefix;
    }
    if (suffix_end > suffix) {
        strncpy(p, suffix, suffix_end - suffix);
        p += suffix_end - suffix;
    }
    if (insert != NULL) {
        /* Note: assume insert does not have leading or trailing '/' */
        strcpy(p, insert);
        p += strlen(insert);
        *p++ = '/';
    }
    strncpy(p, filename, filename_end - filename);
    p += filename_end - filename;
    if (need_slash)
        *p++ = '/';
    *p++ = '\0';

    return (dest);
}

/*
 * The ustar header for the pax extended attributes must have a
 * reasonable name:  SUSv3 requires 'dirname'/PaxHeader.'pid'/'filename'
 * where 'pid' is the PID of the archiving process.  Unfortunately,
 * that makes testing a pain since the output varies for each run,
 * so I'm sticking with the simpler 'dirname'/PaxHeader/'filename'
 * for now.  (Someday, I'll make this settable.  Then I can use the
 * SUS recommendation as default and test harnesses can override it
 * to get predictable results.)
 *
 * Joerg Schilling has argued that this is unnecessary because, in
 * practice, if the pax extended attributes get extracted as regular
 * files, noone is going to bother reading those attributes to
 * manually restore them.  Based on this, 'star' uses
 * /tmp/PaxHeader/'basename' as the ustar header name.  This is a
 * tempting argument, in part because it's simpler than the SUSv3
 * recommendation, but I'm not entirely convinced.  I'm also
 * uncomfortable with the fact that "/tmp" is a Unix-ism.
 *
 * The following routine leverages build_ustar_entry_name() above and
 * so is simpler than you might think.  It just needs to provide the
 * additional path element and handle a few pathological cases).
 */
static char *
build_pax_attribute_name(char *dest, const char *src)
{
    char buff[64];
    const char *p;

    /* Handle the null filename case. */
    if (src == NULL || *src == '\0') {
        strcpy(dest, "PaxHeader/blank");
        return (dest);
    }

    /* Prune final '/' and other unwanted final elements. */
    p = src + strlen(src);
    for (;;) {
        /* Ends in "/", remove the '/' */
        if (p > src && p[-1] == '/') {
            --p;
            continue;
        }
        /* Ends in "/.", remove the '.' */
        if (p > src + 1 && p[-1] == '.'
            && p[-2] == '/') {
            --p;
            continue;
        }
        break;
    }

    /* Pathological case: After above, there was nothing left.
     * This includes "/." "/./." "/.//./." etc. */
    if (p == src) {
        strcpy(dest, "/PaxHeader/rootdir");
        return (dest);
    }

    /* Convert unadorned "." into a suitable filename. */
    if (*src == '.' && p == src + 1) {
        strcpy(dest, "PaxHeader/currentdir");
        return (dest);
    }

    /*
     * TODO: Push this string into the 'pax' structure to avoid
     * recomputing it every time.  That will also open the door
     * to having clients override it.
     */
#if HAVE_GETPID && 0  /* Disable this for now; see above comment. */
    sprintf(buff, "PaxHeader.%d", getpid());
#else
    /* If the platform can't fetch the pid, don't include it. */
    strcpy(buff, "PaxHeader");
#endif
    /* General case: build a ustar-compatible name adding "/PaxHeader/". */
    build_ustar_entry_name(dest, src, p - src, buff);

    return (dest);
}

/* Write two null blocks for the end of archive */
static int
archive_write_pax_finish(struct archive_write *a)
{
    struct pax *pax;
    int r;

    if (a->compressor.write == NULL)
        return (ARCHIVE_OK);

    pax = (struct pax *)a->format_data;
    r = write_nulls(a, 512 * 2);
    return (r);
}

static int
archive_write_pax_destroy(struct archive_write *a)
{
    struct pax *pax;

    pax = (struct pax *)a->format_data;
    if (pax == NULL)
        return (ARCHIVE_OK);

    archive_string_free(&pax->pax_header);
    free(pax);
    a->format_data = NULL;
    return (ARCHIVE_OK);
}

static int
archive_write_pax_finish_entry(struct archive_write *a)
{
    struct pax *pax;
    int ret;

    pax = (struct pax *)a->format_data;
    ret = write_nulls(a, pax->entry_bytes_remaining + pax->entry_padding);
    pax->entry_bytes_remaining = pax->entry_padding = 0;
    return (ret);
}

static int
write_nulls(struct archive_write *a, size_t padding)
{
    int ret, to_write;

    while (padding > 0) {
        to_write = padding < a->null_length ? padding : a->null_length;
        ret = (a->compressor.write)(a, a->nulls, to_write);
        if (ret != ARCHIVE_OK)
            return (ret);
        padding -= to_write;
    }
    return (ARCHIVE_OK);
}

static ssize_t
archive_write_pax_data(struct archive_write *a, const void *buff, size_t s)
{
    struct pax *pax;
    int ret;

    pax = (struct pax *)a->format_data;
    if (s > pax->entry_bytes_remaining)
        s = pax->entry_bytes_remaining;

    ret = (a->compressor.write)(a, buff, s);
    pax->entry_bytes_remaining -= s;
    if (ret == ARCHIVE_OK)
        return (s);
    else
        return (ret);
}

static int
has_non_ASCII(const wchar_t *wp)
{
    if (wp == NULL)
        return (1);
    while (*wp != L'\0' && *wp < 128)
        wp++;
    return (*wp != L'\0');
}

/*
 * Used by extended attribute support; encodes the name
 * so that there will be no '=' characters in the result.
 */
static char *
url_encode(const char *in)
{
    const char *s;
    char *d;
    int out_len = 0;
    char *out;

    for (s = in; *s != '\0'; s++) {
        if (*s < 33 || *s > 126 || *s == '%' || *s == '=')
            out_len += 3;
        else
            out_len++;
    }

    out = (char *)malloc(out_len + 1);
    if (out == NULL)
        return (NULL);

    for (s = in, d = out; *s != '\0'; s++) {
        /* encode any non-printable ASCII character or '%' or '=' */
        if (*s < 33 || *s > 126 || *s == '%' || *s == '=') {
            /* URL encoding is '%' followed by two hex digits */
            *d++ = '%';
            *d++ = "0123456789ABCDEF"[0x0f & (*s >> 4)];
            *d++ = "0123456789ABCDEF"[0x0f & *s];
        } else {
            *d++ = *s;
        }
    }
    *d = '\0';
    return (out);
}

/*
 * Encode a sequence of bytes into a C string using base-64 encoding.
 *
 * Returns a null-terminated C string allocated with malloc(); caller
 * is responsible for freeing the result.
 */
static char *
base64_encode(const char *s, size_t len)
{
    static const char digits[64] =
        { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
          'P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d',
          'e','f','g','h','i','j','k','l','m','n','o','p','q','r','s',
          't','u','v','w','x','y','z','0','1','2','3','4','5','6','7',
          '8','9','+','/' };
    int v;
    char *d, *out;

    /* 3 bytes becomes 4 chars, but round up and allow for trailing NUL */
    out = (char *)malloc((len * 4 + 2) / 3 + 1);
    if (out == NULL)
        return (NULL);
    d = out;

    /* Convert each group of 3 bytes into 4 characters. */
    while (len >= 3) {
        v = (((int)s[0] << 16) & 0xff0000)
            | (((int)s[1] << 8) & 0xff00)
            | (((int)s[2]) & 0x00ff);
        s += 3;
        len -= 3;
        *d++ = digits[(v >> 18) & 0x3f];
        *d++ = digits[(v >> 12) & 0x3f];
        *d++ = digits[(v >> 6) & 0x3f];
        *d++ = digits[(v) & 0x3f];
    }
    /* Handle final group of 1 byte (2 chars) or 2 bytes (3 chars). */
    switch (len) {
    case 0: break;
    case 1:
        v = (((int)s[0] << 16) & 0xff0000);
        *d++ = digits[(v >> 18) & 0x3f];
        *d++ = digits[(v >> 12) & 0x3f];
        break;
    case 2:
        v = (((int)s[0] << 16) & 0xff0000)
            | (((int)s[1] << 8) & 0xff00);
        *d++ = digits[(v >> 18) & 0x3f];
        *d++ = digits[(v >> 12) & 0x3f];
        *d++ = digits[(v >> 6) & 0x3f];
        break;
    }
    /* Add trailing NUL character so output is a valid C string. */
    *d++ = '\0';
    return (out);
}
