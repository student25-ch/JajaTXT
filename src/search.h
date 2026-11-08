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

#ifndef SEARCH_H
#define SEARCH_H
#include <sys/types.h>
#include "reader.h"

typedef struct { off_t offset; unsigned long long line; } SearchHit;
typedef struct SearchCtx SearchCtx;
typedef void (*SearchProgress)(double pct, void *ud);

SearchCtx *search_new(Reader *r, const char *utf8_needle);
void search_free(SearchCtx *s);
size_t search_needle_len(const SearchCtx *s);
int search_next(SearchCtx *s, off_t from, SearchHit *hit, SearchProgress cb, void *ud);

#endif
