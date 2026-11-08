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

#include "ui.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

static struct termios orig;
static int raw_active = 0;

void ui_init(void) {
    if (!isatty(STDIN_FILENO)) return;
    if (tcgetattr(STDIN_FILENO, &orig) < 0) return;
    struct termios raw = orig;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_active = 1;
    printf("\033[?1049h");
    fflush(stdout);
}

void ui_shutdown(void) {
    if (raw_active) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
        printf("\033[?1049l");
        fflush(stdout);
    }
}

void ui_get_size(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0 && ws.ws_col > 0) {
        *rows = ws.ws_row; *cols = ws.ws_col;
    } else { *rows = 24; *cols = 80; }
}

void ui_clear(void) { printf("\033[2J\033[H"); }
void ui_move(int y, int x) { printf("\033[%d;%dH", y + 1, x + 1); }
void ui_flush(void) { fflush(stdout); }

int ui_wait_key(void) {
    unsigned char c = 0;
    if (read(STDIN_FILENO, &c, 1) <= 0) return 'q';
    return c;
}
