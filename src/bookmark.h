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

#ifndef BOOKMARK_H
#define BOOKMARK_H
#include <sys/types.h>
#include "reader.h"

#define BM_NAME_LEN 64

typedef struct {
    off_t offset;
    unsigned long long line;
    char name[BM_NAME_LEN];
} Bookmark;

typedef struct {
    char path[512];
    Encoding enc;
    off_t file_size;
    Bookmark *v;
    size_t n, cap;
} BookmarkStore;

BookmarkStore *bm_open(const char *path, Encoding enc, off_t file_size);
void bm_close(BookmarkStore *b);
size_t bm_count(const BookmarkStore *b);
const Bookmark *bm_get(const BookmarkStore *b, size_t i);
int bm_set_last(BookmarkStore *b, off_t off, unsigned long long line);
int bm_add(BookmarkStore *b, off_t off, unsigned long long line, const char *name);
int bm_remove(BookmarkStore *b, size_t i);
int bm_save(BookmarkStore *b);

#endif
