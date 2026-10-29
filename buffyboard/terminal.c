/**
 * Copyright 2021 Johannes Marbach
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "terminal.h"

#include "../shared/indev.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/kd.h>
#include <linux/vt.h>

#include <sys/ioctl.h>


/**
 * Static variables
 */

static int32_t _tty_size;
static int32_t _h_display_size;
static int32_t _v_display_size;


/**
 * Public functions
 */

void bb_terminal_init(int32_t tty_size, int32_t h_display_size, int32_t v_display_size) {
    _tty_size = tty_size;
    _h_display_size = h_display_size;
    _v_display_size = v_display_size;
}

void bb_terminal_shrink_current() {
    int fd = open("/dev/tty0", O_RDONLY|O_NOCTTY);
    if (fd < 0)
        return;

    /* KDFONTOP returns EINVAL if we are not in the text mode,
        so we can skip this check */
/*
    int mode;
    if (ioctl(fd, KDGETMODE, &mode) != 0)
        goto end;

    if (mode != KD_TEXT)
        goto end;
*/
    struct console_font_op cfo = {
        .op = KD_FONT_OP_GET,
        .width = UINT_MAX,
        .height = UINT_MAX,
        .charcount = UINT_MAX,
        .data = NULL
    };
    if (ioctl(fd, KDFONTOP, &cfo) != 0)
        goto end;

    struct winsize size;
    size.ws_row = _tty_size / cfo.height;
    size.ws_col = _h_display_size / cfo.width;
    ioctl(fd, TIOCSWINSZ, &size);
end:
    close(fd);
}

void bb_terminal_reset_all() {
    int fd = open("/dev/tty0", O_RDONLY|O_NOCTTY);
    if (fd < 0)
        return;

    struct vt_stat state;
    int r = ioctl(fd, VT_GETSTATE, &state);

    close(fd);

    if (r != 0)
        return;

    char buffer[sizeof("/dev/tty") + 2];
    unsigned short mask = state.v_state >> 1;
    unsigned int tty = 1;

    for (; mask; mask >>= 1, tty++) {
        if (mask & 0x01) {
            sprintf(buffer, "/dev/tty%u", tty);
            int tty_fd = open(buffer, O_RDONLY|O_NOCTTY);
            if (tty_fd < 0)
                continue;

            /* KDFONTOP returns EINVAL if we are not in the text mode,
               so we can skip this check */
/*
            int mode;
            if (ioctl(tty_fd, KDGETMODE, &mode) != 0)
                goto end;

            if (mode != KD_TEXT)
                goto end;
*/
            struct console_font_op cfo = {
                .op = KD_FONT_OP_GET,
                .width = UINT_MAX,
                .height = UINT_MAX,
                .charcount = UINT_MAX,
                .data = NULL
            };
            if (ioctl(tty_fd, KDFONTOP, &cfo) != 0)
                goto end;

            struct winsize size;
            size.ws_row = _v_display_size / cfo.height;
            size.ws_col = _h_display_size / cfo.width;
            ioctl(tty_fd, TIOCSWINSZ, &size);
end:
            close(tty_fd);
        }
    }
}

bool bb_terminal_is_busy(unsigned int tty) {
    /* We can use any other terminal than `tty` here. */
    int fd = open(tty == 1? "/dev/tty2" : "/dev/tty1", O_RDONLY|O_NOCTTY);
    if (fd < 0)
        return false;

    struct vt_stat state;
    int r = ioctl(fd, VT_GETSTATE, &state);

    close(fd);

    if (r != 0)
        return false;

    return (state.v_state >> tty) & 0x01;
}

void bb_terminal_on_mode_changed() {
    int fd = open("/dev/tty0", O_RDONLY|O_NOCTTY);
    if (fd < 0)
        return;

    int mode;
    if (ioctl(fd, KDGETMODE, &mode) != 0)
        goto end;

    if (mode == KD_TEXT)
        bbx_indev_resume();
    else
        bbx_indev_suspend();

end:
    close(fd);
}
