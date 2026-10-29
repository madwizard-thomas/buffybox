/**
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ebpf.h"

#include "terminal.h"

#include "../shared/log.h"

#include <linux/bpf.h>
#include <linux/version.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <assert.h>
#include <unistd.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)

/**
 * Public functions
 */

uint8_t bb_ebpf_init(int fd_epoll) {
    static_assert(sizeof("buffyboard") <= BPF_OBJ_NAME_LEN);

    union bpf_attr attr_map = {
        .map_type = BPF_MAP_TYPE_RINGBUF,
        .map_name = {'b', 'u', 'f', 'f', 'y', 'b', 'o', 'a', 'r', 'd'},
        .max_entries = 4096
    };
    int fd_map = syscall(SYS_bpf, BPF_MAP_CREATE, &attr_map, sizeof(attr_map));
    if (fd_map < 0) {
        bbx_log(BBX_LOG_LEVEL_WARNING, "BPF_MAP_CREATE is failed.");
        return 0;
    }

    struct bpf_insn hook[] = {
        /* { .code, .dst_reg, .src_reg, .off, .imm } */
        { BPF_LD | BPF_DW | BPF_IMM, BPF_REG_1, BPF_PSEUDO_MAP_FD, 0, fd_map },
        { 0, 0, 0, 0, 0 },
        { BPF_ALU64 | BPF_MOV | BPF_K, BPF_REG_2, 0, 0, 1 },
        { BPF_ALU64 | BPF_MOV | BPF_K, BPF_REG_3, 0, 0, 0 },
        { BPF_JMP | BPF_CALL | BPF_K, 0, 0, 0, BPF_FUNC_ringbuf_reserve },
        { BPF_JMP | BPF_JEQ | BPF_K, BPF_REG_0, 0, 3, 0},

        { BPF_ALU64 | BPF_MOV | BPF_X, BPF_REG_1, BPF_REG_0, 0, 0 },
        { BPF_ALU64 | BPF_MOV | BPF_K, BPF_REG_2, 0, 0, BPF_RB_FORCE_WAKEUP },
        { BPF_JMP | BPF_CALL | BPF_K, 0, 0, 0, BPF_FUNC_ringbuf_discard },

        { BPF_ALU64 | BPF_MOV | BPF_K, BPF_REG_0, 0, 0, 0 },
        { BPF_JMP | BPF_EXIT, 0, 0, 0, 0 }
    };

    union bpf_attr attr_prog = {
        .prog_name = {'b', 'u', 'f', 'f', 'y', 'b', 'o', 'a', 'r', 'd'},
        .prog_type = BPF_PROG_TYPE_KPROBE,
        .expected_attach_type = BPF_TRACE_KPROBE_MULTI,
        .insns = (__u64) hook,
        .insn_cnt = sizeof(hook) / sizeof(hook[0]),
        .license = (__u64) "GPL-3.0-or-later"
    };
    int fd_hook = syscall(SYS_bpf, BPF_PROG_LOAD, &attr_prog, sizeof(attr_prog));
    if (fd_hook < 0) {
        bbx_log(BBX_LOG_LEVEL_WARNING, "BPF_PROG_LOAD is failed.");
        goto failure_hook;
    }

    const char* kernel_symbols[] = {"do_blank_screen", "do_unblank_screen"};

    union bpf_attr attr_link = {
        .link_create.prog_fd = fd_hook,
        .link_create.attach_type = BPF_TRACE_KPROBE_MULTI,
        .link_create.kprobe_multi.cnt = sizeof(kernel_symbols) / sizeof(kernel_symbols[0]),
        .link_create.kprobe_multi.syms = (__u64) kernel_symbols,
        .link_create.kprobe_multi.flags = BPF_F_KPROBE_MULTI_RETURN
    };
    int fd_link = syscall(SYS_bpf, BPF_LINK_CREATE, &attr_link, sizeof(attr_link));
    if (fd_link < 0) {
        bbx_log(BBX_LOG_LEVEL_WARNING, "BPF_LINK_CREATE is failed.");
        goto failure_link;
    }

    uint8_t ret = 0;

    struct epoll_event event;
    event.events = EPOLLIN|EPOLLET;
    event.data.ptr = __extension__ (void*) bb_terminal_on_mode_changed;

    int r = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_map, &event);
    if (r == -1) {
        bbx_log(BBX_LOG_LEVEL_WARNING, "EPOLL_CTL_ADD for eBPF is failed.");
        goto failure_epoll;
    }
    ret++;

    return ret;


failure_epoll:
    close(fd_link);
failure_link:
    close(fd_hook);
failure_hook:
    close(fd_map);
    return 0;
}

#else

uint8_t bb_ebpf_init(int fd_epoll) {
    (void) fd_epoll;
    return 0;
}

#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
