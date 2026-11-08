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

#include "bookmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
    char magic[8]; uint8_t enc; uint8_t pad[7];
    uint64_t file_size; uint64_t count;
} BmHeader;

typedef struct __attribute__((packed)) {
    uint64_t offset;
    uint64_t line;
    char name[BM_NAME_LEN];
} BmEntry;

BookmarkStore *bm_open(const char *path, Encoding enc, off_t file_size) {
    BookmarkStore *b = calloc(1, sizeof(*b));
    if (!b) return NULL;
    snprintf(b->path, sizeof(b->path), "%s", path);
    b->enc = enc;
    b->file_size = file_size;
    FILE *f = fopen(path, "rb");
    if (!f) return b;
    BmHeader h;
    if (fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return b; }
    if (memcmp(h.magic, "BTMARK01", 8) || h.enc != (uint8_t)enc || h.file_size != (uint64_t)file_size) {
        fclose(f); return b;
    }
    for (uint64_t i = 0; i < h.count; i++) {
        BmEntry e;
        if (fread(&e, sizeof(e), 1, f) != 1) break;
        if (b->n >= b->cap) {
            size_t nc = b->cap ? b->cap * 2 : 8;
            Bookmark *nv = realloc(b->v, nc * sizeof(Bookmark));
            if (!nv) break;
            b->v = nv; b->cap = nc;
        }
        b->v[b->n].offset = (off_t)e.offset;
        b->v[b->n].line = e.line;
        memcpy(b->v[b->n].name, e.name, BM_NAME_LEN);
        b->v[b->n].name[BM_NAME_LEN-1] = 0;
        b->n++;
    }
    fclose(f);
    return b;
}

void bm_close(BookmarkStore *b) { if (b) { free(b->v); free(b); } }
size_t bm_count(const BookmarkStore *b) { return b->n; }
const Bookmark *bm_get(const BookmarkStore *b, size_t i) { return &b->v[i]; }

static int ensure_cap(BookmarkStore *b, size_t need) {
    if (b->cap >= need) return 0;
    size_t nc = b->cap ? b->cap : 8;
    while (nc < need) nc *= 2;
    Bookmark *nv = realloc(b->v, nc * sizeof(Bookmark));
    if (!nv) return -1;
    b->v = nv; b->cap = nc;
    return 0;
}

int bm_set_last(BookmarkStore *b, off_t off, unsigned long long line) {
    if (ensure_cap(b, 1) != 0) return -1;
    if (b->n == 0) b->n = 1;
    b->v[0].offset = off;
    b->v[0].line = line;
    snprintf(b->v[0].name, BM_NAME_LEN, "__last__");
    return 0;
}

int bm_add(BookmarkStore *b, off_t off, unsigned long long line, const char *name) {
    if (ensure_cap(b, b->n + 1) != 0) return -1;
    b->v[b->n].offset = off;
    b->v[b->n].line = line;
    snprintf(b->v[b->n].name, BM_NAME_LEN, "%s", (name && *name) ? name : "书签");
    b->n++;
    return 0;
}

int bm_remove(BookmarkStore *b, size_t i) {
    if (i >= b->n) return -1;
    for (size_t k = i; k + 1 < b->n; k++) b->v[k] = b->v[k+1];
    b->n--;
    return 0;
}

int bm_save(BookmarkStore *b) {
    FILE *f = fopen(b->path, "wb");
    if (!f) return -1;
    BmHeader h; memset(&h, 0, sizeof(h));
    memcpy(h.magic, "BTMARK01", 8);
    h.enc = (uint8_t)b->enc; h.file_size = (uint64_t)b->file_size; h.count = (uint64_t)b->n;
    if (fwrite(&h, sizeof(h), 1, f) != 1) { fclose(f); return -1; }
    for (size_t i = 0; i < b->n; i++) {
        BmEntry e; memset(&e, 0, sizeof(e));
        e.offset = (uint64_t)b->v[i].offset;
        e.line = b->v[i].line;
        memcpy(e.name, b->v[i].name, BM_NAME_LEN);
        if (fwrite(&e, sizeof(e), 1, f) != 1) { fclose(f); return -1; }
    }
    fclose(f);
    return 0;
}
