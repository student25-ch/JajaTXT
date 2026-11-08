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

#ifndef INDEX_H
#define INDEX_H
#include <sys/types.h>
#include "reader.h"

typedef struct SparseIndex SparseIndex;

SparseIndex *sindex_new(void);
size_t sindex_count(const SparseIndex *ix);
off_t sindex_offset_at(const SparseIndex *ix, size_t i);
int sindex_build(Reader *r, SparseIndex *ix, void (*progress)(double, void *), void *ud);
#endif
