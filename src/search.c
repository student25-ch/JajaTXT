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

#include "search.h"
#include <stdlib.h>
#include <string.h>

#define SEARCH_CHUNK (4 * 1024 * 1024)

struct SearchCtx {
    Reader *r;
    char *needle;
    size_t nlen;
};

SearchCtx *search_new(Reader *r, const char *utf8_needle) {
    if (!r || !utf8_needle || !*utf8_needle) return NULL;
    SearchCtx *s = calloc(1, sizeof(*s));
    if (!s) return NULL;
    s->r = r;
    s->needle = reader_convert_from_utf8(r, utf8_needle, &s->nlen);
    if (!s->needle || s->nlen == 0) { search_free(s); return NULL; }
    return s;
}

void search_free(SearchCtx *s) { if (s) { free(s->needle); free(s); } }
size_t search_needle_len(const SearchCtx *s) { return s->nlen; }

static off_t find_bytes(const char *buf, size_t len, const char *needle, size_t nlen) {
    if (nlen == 0 || nlen > len) return -1;
    const char *p = buf, *end = buf + len - nlen + 1;
    while (p < end) {
        const char *q = memchr(p, needle[0], (size_t)(end - p));
        if (!q) return -1;
        if (memcmp(q, needle, nlen) == 0) return (off_t)(q - buf);
        p = q + 1;
    }
    return -1;
}

int search_next(SearchCtx *s, off_t from, SearchHit *hit, SearchProgress cb, void *ud) {
    const char *data = reader_data(s->r);
    off_t size = reader_size(s->r);
    if (from < 0) from = 0;
    if (from + (off_t)s->nlen > size) return 0;
    off_t pos = from;
    while (pos < size) {
        size_t want = SEARCH_CHUNK + s->nlen - 1;
        if ((off_t)want > size - pos) want = (size_t)(size - pos);
        off_t idx = find_bytes(data + pos, want, s->needle, s->nlen);
        if (idx >= 0) {
            hit->offset = pos + idx;
            hit->line = 0;
            return 1;
        }
        if ((off_t)want < SEARCH_CHUNK + (off_t)s->nlen - 1) break;
        pos += SEARCH_CHUNK;
        if (cb) cb((double)pos / (double)size, ud);
    }
    if (cb) cb(1.0, ud);
    return 0;
}
