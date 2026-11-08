/*
 * Copyright (C) 2026 student25-ch
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iconv.h>

struct Reader {
    int fd;
    const char *map_data;
    off_t map_size;
    const char *data;
    off_t size;
    Encoding enc;
    iconv_t cd;
};

static int nl_le(unsigned char a, unsigned char b) { return a == 0x0A && b == 0x00; }
static int nl_be(unsigned char a, unsigned char b) { return a == 0x00 && b == 0x0A; }

static Encoding detect_encoding(const char *d, off_t n, int *bom_skip) {
    *bom_skip = 0;
    if (n >= 3 && (unsigned char)d[0] == 0xEF && (unsigned char)d[1] == 0xBB && (unsigned char)d[2] == 0xBF) {
        *bom_skip = 3; return ENC_UTF8;
    }
    if (n >= 2 && (unsigned char)d[0] == 0xFF && (unsigned char)d[1] == 0xFE) {
        *bom_skip = 2; return ENC_UTF16LE;
    }
    if (n >= 2 && (unsigned char)d[0] == 0xFE && (unsigned char)d[1] == 0xFF) {
        *bom_skip = 2; return ENC_UTF16BE;
    }
    off_t probe = n < 4096 ? n : 4096;
    int zero_odd = 0, nonzero_even = 0, zero_even = 0, nonzero_odd = 0;
    for (off_t i = 0; i + 1 < probe; i += 2) {
        if (d[i+1] == 0) zero_odd++;
        if (d[i] != 0) nonzero_even++;
        if (d[i] == 0) zero_even++;
        if (d[i+1] != 0) nonzero_odd++;
    }
    off_t pairs = probe / 2;
    if (pairs > 0) {
        if (zero_odd * 100 / pairs > 30 && nonzero_even * 100 / pairs > 30) return ENC_UTF16LE;
        if (zero_even * 100 / pairs > 30 && nonzero_odd * 100 / pairs > 30) return ENC_UTF16BE;
    }
    int utf8_ok = 1;
    for (off_t i = 0; i < probe; ) {
        unsigned char c = (unsigned char)d[i];
        int len;
        if (c < 0x80) len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        else { utf8_ok = 0; break; }
        if (i + len > probe) break;
        for (int k = 1; k < len; k++)
            if (((unsigned char)d[i+k] & 0xC0) != 0x80) { utf8_ok = 0; break; }
        if (!utf8_ok) break;
        i += len;
    }
    return utf8_ok ? ENC_UTF8 : ENC_GBK;
}

const char *reader_encoding_name(Encoding e) {
    switch (e) {
        case ENC_UTF8: return "UTF-8";
        case ENC_GBK: return "GBK/GB18030";
        case ENC_UTF16LE: return "UTF-16LE";
        case ENC_UTF16BE: return "UTF-16BE";
        default: return "未知";
    }
}

Reader *reader_open(const char *path) {
    Reader *r = calloc(1, sizeof(*r));
    if (!r) return NULL;
    r->fd = -1;
    r->cd = (iconv_t)-1;
    r->fd = open(path, O_RDONLY);
    if (r->fd < 0) { free(r); return NULL; }
    struct stat st;
    if (fstat(r->fd, &st) < 0) { close(r->fd); free(r); return NULL; }
    r->map_size = st.st_size;
    if (r->map_size <= 0) {
        r->map_data = ""; r->data = ""; r->size = 0; r->enc = ENC_UTF8;
        return r;
    }
    void *m = mmap(NULL, (size_t)r->map_size, PROT_READ, MAP_PRIVATE, r->fd, 0);
    if (m == MAP_FAILED) { close(r->fd); free(r); return NULL; }
    r->map_data = (const char *)m;
    int bom = 0;
    r->enc = detect_encoding(r->map_data, r->map_size, &bom);
    r->data = r->map_data + bom;
    r->size = r->map_size - bom;

    const char *from = NULL;
    switch (r->enc) {
        case ENC_GBK: from = "GB18030"; break;
        case ENC_UTF16LE: from = "UTF-16LE"; break;
        case ENC_UTF16BE: from = "UTF-16BE"; break;
        default: break;
    }
    if (from) {
        r->cd = iconv_open("UTF-8", from);
        if (r->cd == (iconv_t)-1 && r->enc == ENC_GBK)
            r->cd = iconv_open("UTF-8", "GBK");
        if (r->cd == (iconv_t)-1) {
            fprintf(stderr, "警告: 无法打开 iconv，将按 UTF-8 原样显示\n");
            r->enc = ENC_UTF8;
        }
    }
    return r;
}

void reader_close(Reader *r) {
    if (!r) return;
    if (r->cd != (iconv_t)-1) iconv_close(r->cd);
    if (r->map_data && r->map_size > 0) munmap((void *)r->map_data, (size_t)r->map_size);
    if (r->fd >= 0) close(r->fd);
    free(r);
}

const char *reader_data(const Reader *r) { return r->data; }
off_t reader_size(const Reader *r) { return r->size; }
Encoding reader_encoding(const Reader *r) { return r->enc; }

off_t reader_next_line(Reader *r, off_t pos) {
    if (r->enc == ENC_UTF16LE || r->enc == ENC_UTF16BE) {
        off_t p = pos;
        while (p + 1 < r->size) {
            unsigned char a = (unsigned char)r->data[p], b = (unsigned char)r->data[p+1];
            int nl = (r->enc == ENC_UTF16LE) ? nl_le(a,b) : nl_be(a,b);
            if (nl) return p + 2;
            p += 2;
        }
        return r->size;
    }
    off_t p = pos;
    while (p < r->size && r->data[p] != '\n') p++;
    return (p < r->size) ? p + 1 : r->size;
}

off_t reader_line_start(const Reader *r, off_t pos) {
    if (r->size <= 0) return 0;
    if (pos >= r->size) pos = r->size - 1;
    if (r->enc == ENC_UTF16LE || r->enc == ENC_UTF16BE) {
        if (pos & 1) pos--;
        while (pos >= 2) {
            unsigned char a = (unsigned char)r->data[pos-2], b = (unsigned char)r->data[pos-1];
            int nl = (r->enc == ENC_UTF16LE) ? nl_le(a,b) : nl_be(a,b);
            if (nl) break;
            pos -= 2;
        }
        return pos;
    }
    while (pos > 0 && r->data[pos-1] != '\n') pos--;
    return pos;
}

size_t reader_line(Reader *r, off_t pos, char *buf, size_t cap, off_t *next) {
    off_t p = pos, end;
    if (r->enc == ENC_UTF16LE || r->enc == ENC_UTF16BE) {
        int found = 0;
        while (p + 1 < r->size) {
            unsigned char a = (unsigned char)r->data[p], b = (unsigned char)r->data[p+1];
            int nl = (r->enc == ENC_UTF16LE) ? nl_le(a,b) : nl_be(a,b);
            if (nl) { found = 1; break; }
            p += 2;
        }
        end = p;
        if (end >= pos + 2) {
            unsigned char a = (unsigned char)r->data[end-2], b = (unsigned char)r->data[end-1];
            int cr = (r->enc == ENC_UTF16LE) ? (a==0x0D && b==0x00) : (a==0x00 && b==0x0D);
            if (cr) end -= 2;
        }
        *next = found ? p + 2 : r->size;
    } else {
        while (p < r->size && r->data[p] != '\n') p++;
        end = p;
        if (end > pos && r->data[end-1] == '\r') end--;
        *next = (p < r->size) ? p + 1 : r->size;
    }
    if (r->enc == ENC_UTF8) {
        off_t n = end - pos;
        if (n > (off_t)cap) n = (off_t)cap;
        if (n > 0) memcpy(buf, r->data + pos, (size_t)n);
        return (size_t)n;
    }
    char *in = (char *)r->data + pos;
    size_t inleft = (size_t)(end - pos);
    char *out = buf;
    size_t outleft = cap;
    iconv(r->cd, NULL, NULL, NULL, NULL);
    iconv(r->cd, &in, &inleft, &out, &outleft);
    return cap - outleft;
}

char *reader_convert_from_utf8(Reader *r, const char *utf8, size_t *outlen) {
    *outlen = 0;
    if (!utf8 || !*utf8) return NULL;
    if (r->enc == ENC_UTF8) {
        size_t n = strlen(utf8);
        char *out = malloc(n);
        if (!out) return NULL;
        memcpy(out, utf8, n);
        *outlen = n;
        return out;
    }
    const char *to = NULL;
    switch (r->enc) {
        case ENC_GBK: to = "GB18030"; break;
        case ENC_UTF16LE: to = "UTF-16LE"; break;
        case ENC_UTF16BE: to = "UTF-16BE"; break;
        default: return NULL;
    }
    iconv_t cd = iconv_open(to, "UTF-8");
    if (cd == (iconv_t)-1) return NULL;
    size_t inlen = strlen(utf8);
    size_t cap = inlen * 4 + 8;
    char *out = malloc(cap);
    if (!out) { iconv_close(cd); return NULL; }
    char *in = (char *)utf8, *op = out;
    size_t oleft = cap;
    iconv(cd, &in, &inlen, &op, &oleft);
    iconv_close(cd);
    *outlen = cap - oleft;
    return out;
}
