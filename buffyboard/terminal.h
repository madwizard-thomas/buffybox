/**
 * Copyright 2021 Johannes Marbach
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef BB_TERMINAL_H
#define BB_TERMINAL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Prepare for resizing terminals.
 * 
 * @param tty_size vertical size of the tty area in pixels
 * @param h_display_size horizontal resolution of the display in pixels
 * @param v_display_size vertical resolution of the display in pixels
 */
void bb_terminal_init(int32_t tty_size, int32_t h_display_size, int32_t v_display_size);

/**
 * Shrink the height of the active terminal.
 */
void bb_terminal_shrink_current();

/**
 * Re-maximise the size of all active terminals.
 */
void bb_terminal_reset_all();

/**
 * Check whether the terminal is opened by some process.
 */
bool bb_terminal_is_busy(unsigned int tty);

/**
 * Suspend/resume input processing in KD_GRAPHICS/KD_TEXT modes.
 */
void bb_terminal_on_mode_changed();

#endif /* BB_TERMINAL_H */
