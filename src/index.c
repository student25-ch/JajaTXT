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

#include <string.h>
#include <stdint.h>

#define SKIP_LINES 1000ULL

typedef struct __attribute__((packed)) {
    char magic[8]; uint8_t enc; uint8_t pad[7];
    uint64_t file_size; uint64_t count;
SparseIndex *sindex_new(void) { return calloc(1, sizeof(SparseIndex)); }
void sindex_free(SparseIndex *ix) { if (ix) { free(ix->v); free(ix); } }
void sindex_reset(SparseIndex *ix) { ix->n = 0; }

static void grow(SparseIndex *ix) {
    if (ix->n < ix->cap) return;
    size_t nc = ix->cap ? ix->cap * 2 : 1024;
    Entry *nv = realloc(ix->v, nc * sizeof(Entry));
    if (!nv) { perror("realloc"); exit(1); }
    ix->v = nv; ix->cap = nc;
}

void sindex_add(SparseIndex *ix, off_t off, unsigned long long line) {
    grow(ix);
    ix->v[ix->n].offset = off;
    ix->v[ix->n].line = line;

long sindex_find_le(const SparseIndex *ix, off_t pos) {
    long lo = 0, hi = (long)ix->n - 1, best = -1;
    while (lo <= hi) {
        long mid = (lo + hi) / 2;
        if (ix->v[mid].offset <= pos) { best = mid; lo = mid + 1; }
        else hi = mid - 1;

int sindex_build(Reader *r, SparseIndex *ix, void (*progress)(double, void *), void *ud) {
    sindex_reset(ix);
    off_t size = reader_size(r), pos = 0;
    unsigned long long line = 0, counter = 0;
    sindex_add(ix, 0, 0);
    while (pos < size) {
        off_t nxt = reader_next_line(r, pos);
        if (nxt <= pos) break;
        pos = nxt; line++; counter++;
        if (counter >= SKIP_LINES) {
            counter = 0;
            sindex_add(ix, pos, line);
            if (progress) progress((double)pos / (double)size, ud);
        }
    }
    if (progress) progress(1.0, ud);
    return 0;
}

int sindex_save(const SparseIndex *ix, const char *path, Encoding enc, off_t file_size) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    IdxHeader h; memset(&h, 0, sizeof(h));
    memcpy(h.magic, "BTIDX001", 8);
    h.enc = (uint8_t)enc; h.file_size = (uint64_t)file_size; h.count = (uint64_t)ix->n;
    if (fwrite(&h, sizeof(h), 1, f) != 1) { fclose(f); return -1; }
    for (size_t i = 0; i < ix->n; i++) {
        IdxEntry e; e.offset = (uint64_t)ix->v[i].offset; e.line = (uint64_t)ix->v[i].line;
        if (fwrite(&e, sizeof(e), 1, f) != 1) { fclose(f); return -1; }
    }
    fclose(f); return 0;
}

int sindex_load(SparseIndex *ix, const char *path, Encoding enc, off_t file_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    IdxHeader h;
    if (fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return -1; }
    if (memcmp(h.magic, "BTIDX001", 8) || h.enc != (uint8_t)enc || h.file_size != (uint64_t)file_size) {
        fclose(f); return -1;
    }
    sindex_reset(ix);
    for (uint64_t i = 0; i < h.count; i++) {
        IdxEntry e;
        if (fread(&e, sizeof(e), 1, f) != 1) { fclose(f); return -1; }
        sindex_add(ix, (off_t)e.offset, (unsigned long long)e.line);
    }
    return best;
}
    ix->n++;
}
unsigned long long sindex_line_at(const SparseIndex *ix, size_t i) { return ix->v[i].line; }

size_t sindex_count(const SparseIndex *ix) { return ix->n; }
off_t sindex_offset_at(const SparseIndex *ix, size_t i) { return ix->v[i].offset; }
} IdxHeader;
typedef struct __attribute__((packed)) { uint64_t offset, line; } IdxEntry;


typedef struct { off_t offset; unsigned long long line; } Entry;
struct SparseIndex { Entry *v; size_t n, cap; };
#include "index.h"
#include <stdio.h>
#include <stdlib.h>
int sindex_save(const SparseIndex *ix, const char *path, Encoding enc, off_t file_size);
int sindex_load(SparseIndex *ix, const char *path, Encoding enc, off_t file_size);

unsigned long long sindex_line_at(const SparseIndex *ix, size_t i);
long sindex_find_le(const SparseIndex *ix, off_t pos);

void sindex_free(SparseIndex *ix);
void sindex_reset(SparseIndex *ix);
void sindex_add(SparseIndex *ix, off_t off, unsigned long long line);

