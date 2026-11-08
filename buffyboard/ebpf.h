/**
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef BB_EBPF_H
#define BB_EBPF_H

#include <stdint.h>

/**
 * Set up monitoring for the terminal mode (KD_TEXT or KD_GRAPHICS).
 *
 * @param fd_epoll epoll descriptor to add monitored events to
 * @return the number of file descriptors added to fd_epoll
 */
uint8_t bb_ebpf_init(int fd_epoll);

#endif /* BB_EBPF_H */
