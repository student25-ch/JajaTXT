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

#ifndef READER_H
#define READER_H
#include <sys/types.h>

typedef enum { ENC_UNKNOWN = 0, ENC_UTF8, ENC_GBK, ENC_UTF16LE, ENC_UTF16BE } Encoding;

typedef struct Reader Reader;

Reader *reader_open(const char *path);
void reader_close(Reader *r);
const char *reader_data(const Reader *r);
off_t reader_size(const Reader *r);
Encoding reader_encoding(const Reader *r);
const char *reader_encoding_name(Encoding e);

off_t reader_next_line(Reader *r, off_t pos);
off_t reader_line_start(const Reader *r, off_t pos);
size_t reader_line(Reader *r, off_t pos, char *buf, size_t cap, off_t *next);

char *reader_convert_from_utf8(Reader *r, const char *utf8, size_t *outlen);

#endif
