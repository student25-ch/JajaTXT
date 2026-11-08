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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"
#include "index.h"
#include "search.h"
#include "bookmark.h"
#include "ui.h"

static int g_rows = 24, g_cols = 80;
static char g_linebuf[1 << 18];
static int g_last_pct = -1;

static int utf8_len(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}
static int utf8_width(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 1;
    if ((c & 0xF0) == 0xE0) return 2;
    if ((c & 0xF8) == 0xF0) return 2;
    return 1;
}

static int visual_lines(const char *buf, size_t n, int cols) {
    if (n == 0) return 1;
    int lines = 1, x = 0;
    size_t i = 0;
    while (i < n) {
        unsigned char c = (unsigned char)buf[i];
        int len = utf8_len(c), w = utf8_width(c);
        if (x + w > cols) { lines++; x = 0; }
        i += (size_t)len; x += w;
    }
    return lines;
}

static void display_line(const char *buf, size_t n, int cols) {
    size_t i = 0; int x = 0;
#!/usr/bin/env bash
# 一键生成多文件C大TXT阅读器（编码识别 + 稀疏索引 + 搜索 + 书签）并构建
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)/bigtxt"
mkdir -p "$ROOT/src"
cd "$ROOT"

cat > src/reader.h <<'EOF'
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
