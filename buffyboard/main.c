/**
 * Copyright 2021 Johannes Marbach
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "buffyboard.h"
#include "command_line.h"
#include "config.h"
#include "ebpf.h"
#include "sq2lv_layouts.h"
#include "terminal.h"
#include "uinput_device.h"

#include "lvgl/lvgl.h"

#include "../shared/force_feedback.h"
#include "../shared/indev.h"
#include "../shared/log.h"
#include "../shared/theme.h"
#include "../shared/themes.h"
#include "../squeek2lvgl/sq2lv.h"

#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


/**
 * Static variables
 */

bb_cli_opts cli_opts;
bb_config_opts conf_opts;

static int fd_active;
static lv_obj_t *keyboard = NULL;

/**
 * Static prototypes
 */

/**
 * Callback for buttons on a stylus.
 */
static void tablet_tool_button_cb();

/**
 * Handle signals sent to the process.
 *
 * @param signum the signal's number
 */
static void signal_handler(int signum);

/**
 * Handle LV_EVENT_VALUE_CHANGED events from the keyboard widget.
 * 
 * @param event the event object
 */
static void keyboard_value_changed_cb(lv_event_t *event);

/**
 * Emit key down and up events for a key.
 *
 * @param btn_id button index corresponding to the key
 * @param key_down true if a key down event should be emitted
 * @param key_up true if a key up event should be emitted
 */
static void emit_key_events(uint16_t btn_id, bool key_down, bool key_up);

/**
 * Release any previously pressed modifier keys.
 */
static void pop_checked_modifier_keys(void);

/**
 * Shrink the currrent terminal and draw the keyboard.
 */
static void update_terminal();


/**
 * Static functions
 */

static void tablet_tool_button_cb() {
    sq2lv_toggle_fourth_layer(keyboard);
}

static void signal_handler(int signum) {
    switch (signum) {
    case SIGUSR1:
        update_terminal();
        return;
    case SIGALRM:
        bb_terminal_on_mode_changed();
        return;
    default:
        bb_terminal_reset_all();
        exit(0);
    }
}

static void keyboard_value_changed_cb(lv_event_t *event) {
    lv_obj_t *kb = lv_event_get_target(event);

    uint16_t btn_id = lv_buttonmatrix_get_selected_button(kb);
    if (btn_id == LV_BUTTONMATRIX_BUTTON_NONE) {
        return;
    }

    bbx_force_feedback_play();

    if (sq2lv_is_layer_switcher(kb, btn_id)) {
        pop_checked_modifier_keys();
        sq2lv_switch_layer(kb, btn_id);
        return;
    }

    /* Note that the LV_BUTTONMATRIX_CTRL_CHECKED logic is inverted because LV_KEYBOARD_CTRL_BTN_FLAGS already
     * contains LV_BUTTONMATRIX_CTRL_CHECKED. As a result, pressing e.g. CTRL will _un_check the key. To account
     * for this, we invert the meaning of "checked" here and elsewhere in the code. */

    bool is_modifier = sq2lv_is_modifier(keyboard, btn_id);
    bool is_checked = !lv_buttonmatrix_has_button_ctrl(keyboard, btn_id, LV_BUTTONMATRIX_CTRL_CHECKED);

    /* Emit key events. Suppress key up events for modifiers unless they were unchecked. For checked modifiers
     * the key up events are sent with the next non-modifier key press. */
    emit_key_events(btn_id, true, !is_modifier || !is_checked);

    /* Pop any previously checked modifiers when a non-modifier key was pressed */
    if (!is_modifier) {
        pop_checked_modifier_keys();

        /* If sticky_shift is disabled and we're in uppercase mode, switch back to lowercase */
        if (!conf_opts.keyboard.sticky_shift && lv_keyboard_get_mode(keyboard) == LV_KEYBOARD_MODE_TEXT_UPPER) {
            lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        }
    }
}

static void emit_key_events(uint16_t btn_id, bool key_down, bool key_up) {
    int num_scancodes = 0;
    const int *scancodes = sq2lv_get_scancodes(keyboard, btn_id, &num_scancodes);

    if (key_down) {
        /* Emit key down events in forward order */
        for (int i = 0; i < num_scancodes; ++i) {
            bb_uinput_device_emit_key_down(scancodes[i]);
        }
    }

    if (key_up) {
        /* Emit key up events in backward order */
        for (int i = num_scancodes - 1; i >= 0; --i) {
            bb_uinput_device_emit_key_up(scancodes[i]);
        }
    }
}

static void pop_checked_modifier_keys(void) {
    uint32_t num_modifiers = 0;
    const uint32_t *modifier_idxs = sq2lv_get_modifier_indexes(keyboard, &num_modifiers);

    for (uint32_t i = 0; i < num_modifiers; ++i) {
        if (!lv_buttonmatrix_has_button_ctrl(keyboard, modifier_idxs[i], LV_BUTTONMATRIX_CTRL_CHECKED)) {
            emit_key_events(modifier_idxs[i], false, true);
            lv_buttonmatrix_set_button_ctrl(keyboard, modifier_idxs[i], LV_BUTTONMATRIX_CTRL_CHECKED);
        }
    }
}

static void update_terminal() {
    bb_terminal_shrink_current();
    lv_obj_invalidate(keyboard);
}

static void on_new_terminal() {
    pop_checked_modifier_keys();

    if (conf_opts.quirks.ignore_unused_terminals) {
        lseek(fd_active, 0, SEEK_SET);

        char buffer[8];
        ssize_t size = read(fd_active, buffer, sizeof(buffer) - 1);
        if (size <= 0) {
            bbx_log(BBX_LOG_LEVEL_WARNING, "Can't read /sys/class/tty/tty0/active");
            return;
        }
        buffer[size] = 0;

        unsigned int tty;
        if (sscanf(buffer, "tty%u", &tty) != 1) {
            bbx_log(BBX_LOG_LEVEL_WARNING, "Unexpected value of /sys/class/tty/tty0/active");
            return;
        }

        if (!bb_terminal_is_busy(tty)) {
            bbx_log(BBX_LOG_LEVEL_VERBOSE, "Terminal %u isn't used, skip automatic update.", tty);
            return;
        }
    }

    update_terminal();
}

/**
 * Main
 */

int main(int argc, char *argv[]) {
    /* Parse command line options */
    bb_cli_parse_opts(argc, argv, &cli_opts);

    /* Set up log level */
    if (cli_opts.common.verbose) {
        bbx_log_set_level(BBX_LOG_LEVEL_VERBOSE);
    }

    /* Parse config files */
    bb_config_init_opts(&conf_opts);
    bb_config_parse_file("/usr/share/buffyboard/buffyboard.conf", &conf_opts);
    bb_config_parse_directory("/usr/share/buffyboard/buffyboard.conf.d", &conf_opts);
    bb_config_parse_file("/etc/buffyboard.conf", &conf_opts);
    bb_config_parse_directory("/etc/buffyboard.conf.d", &conf_opts);
    bb_config_parse_files(cli_opts.config_files, cli_opts.num_config_files, &conf_opts);

    /* Initialise LVGL and set up logging callback */
    lv_init();
    lv_log_register_print_cb(bbx_log_print_cb);

    /* Initialise display */
    lv_display_t *disp = lv_linux_fbdev_create();
    if (access("/dev/fb0", F_OK) != 0) {
        bbx_log(BBX_LOG_LEVEL_ERROR, "/dev/fb0 is not available");
        return EXIT_FAILURE;
    }
    lv_linux_fbdev_set_file(disp, "/dev/fb0");
    if (conf_opts.quirks.fbdev_force_refresh) {
        lv_linux_fbdev_set_force_refresh(disp, true);
    }
    lv_display_set_physical_resolution(disp,
        lv_display_get_horizontal_resolution(disp),
        lv_display_get_vertical_resolution(disp));

    /* Override display properties with command line options if necessary */
    lv_display_set_offset(disp, cli_opts.common.x_offset, cli_opts.common.y_offset);
    if (cli_opts.common.hor_res > 0 && cli_opts.common.ver_res > 0) {
        lv_display_set_resolution(disp, cli_opts.common.hor_res, cli_opts.common.ver_res);
    }
    if (cli_opts.common.dpi > 0) {
        lv_display_set_dpi(disp, cli_opts.common.dpi);
    }

    /* Set up display rotation */
    lv_display_set_rotation(disp, cli_opts.rotation);

    const int32_t hor_res = lv_display_get_horizontal_resolution(disp);
    const int32_t ver_res = lv_display_get_vertical_resolution(disp);
    const int32_t keyboard_height = ver_res > hor_res ? ver_res / 3 : ver_res / 2;
    const int32_t tty_height = ver_res - keyboard_height;

    switch (cli_opts.rotation) {
    case LV_DISPLAY_ROTATION_0:
    case LV_DISPLAY_ROTATION_180:
        lv_display_set_resolution(disp, hor_res, keyboard_height);
        lv_display_set_offset(disp, 0, cli_opts.rotation == LV_DISPLAY_ROTATION_0? tty_height : 0);
        break;
    case LV_DISPLAY_ROTATION_90:
    case LV_DISPLAY_ROTATION_270:
        lv_display_set_resolution(disp, keyboard_height, hor_res);
        lv_display_set_offset(disp, cli_opts.rotation == LV_DISPLAY_ROTATION_90? tty_height : 0, 0);
        break;
    }

    /* Prepare for terminal resizing and reset */
    bb_terminal_init(tty_height - 8, hor_res, ver_res);

    /* Initialise theme */
    bbx_theme_apply(bbx_themes_themes[conf_opts.theme.default_id]);
    lv_theme_apply(lv_screen_active());

    /* Add keyboard */
    keyboard = lv_keyboard_create(lv_screen_active());
    uint32_t num_keyboard_events = lv_obj_get_event_count(keyboard);
    for(uint32_t i = 0; i < num_keyboard_events; ++i) {
        if(lv_event_dsc_get_cb(lv_obj_get_event_dsc(keyboard, i)) == lv_keyboard_def_event_cb) {
            lv_obj_remove_event(keyboard, i);
            break;
        }
    }
    lv_obj_add_event_cb(keyboard, keyboard_value_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_pos(keyboard, 0, 0);
    lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES);
    bbx_theme_prepare_keyboard(keyboard);

    /* Apply default keyboard layout */
    sq2lv_switch_layout(keyboard, SQ2LV_LAYOUT_TERMINAL_US);

    int fd_epoll = epoll_create1(EPOLL_CLOEXEC);
    if (fd_epoll == -1) {
        perror("epoll_create1() is failed");
        return EXIT_FAILURE;
    }

    /* Open the file to track virtual terminals */
    fd_active = open("/sys/class/tty/tty0/active", O_RDONLY|O_NOCTTY|O_CLOEXEC);
    if (fd_active < 0) {
        perror("Can't open /sys/class/tty/tty0/active");
        return EXIT_FAILURE;
    }

    struct epoll_event event;
    event.events = EPOLLIN|EPOLLET;
    event.data.ptr = __extension__ (void*) on_new_terminal;

    int r = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_active, &event);
    if (r == -1) {
        perror("epoll_ctl() is failed");
        return EXIT_FAILURE;
    }

    /* Attach input devices and start monitoring for new ones */
    struct bbx_indev_opts input_config = {
        .pointer = conf_opts.input.pointer,
        .touchscreen = conf_opts.input.touchscreen,
        .force_feedback = conf_opts.keyboard.haptic_feedback
    };
    if (bbx_indev_init(fd_epoll, &input_config) == 0)
        return EXIT_FAILURE;

    bbx_indev_set_tablet_tool_button_cb(tablet_tool_button_cb);

    /* Set up uinput device */
    if (!bb_uinput_device_init(sq2lv_unique_scancodes, sq2lv_num_unique_scancodes)) {
        return EXIT_FAILURE;
    }

    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGUSR1);
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigmask, NULL);

    /* Set up kernel hooks to track the terminal mode */
    if (bb_ebpf_init(fd_epoll) == 0) {
        // Fallback to polling with a timer
        timer_t id_timer;
        if (timer_create(CLOCK_MONOTONIC, NULL, &id_timer) != -1) {
            struct itimerspec spec = {
                .it_value.tv_sec = 1,
                .it_value.tv_nsec = 0,
                .it_interval.tv_sec = 1,
                .it_interval.tv_nsec = 0
            };
            if (timer_settime(id_timer, 0, &spec, NULL) == -1)
                bbx_log(BBX_LOG_LEVEL_WARNING, "timer_settime() is failed: %s", strerror(errno));
        } else
            bbx_log(BBX_LOG_LEVEL_WARNING, "timer_create() is failed: %s", strerror(errno));
    }

    /* Suspend libinput if the terminal is in KD_GRAPHICS mode */
    bb_terminal_on_mode_changed();

    /* Set signal handlers */
    struct sigaction action;
    action.sa_handler = signal_handler;
    action.sa_flags = 0;
    sigfillset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGALRM);
    sigaction(SIGUSR1, &action, NULL);

    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGUSR1);
    sigaction(SIGALRM, &action, NULL);

    sigemptyset(&sigmask);

    /* Event loop */
    while(1) {
        uint32_t time_till_next = lv_timer_handler();

        int r = epoll_pwait(fd_epoll, &event, 1, time_till_next, &sigmask);
        if (r == 0)
            continue;
        if (r > 0) {
            __extension__ void (*handler)() = event.data.ptr;
            handler();
            continue;
        }
        if (errno == EINTR)
            continue;

        bbx_log(BBX_LOG_LEVEL_ERROR, "epoll_pwait() is failed");
        bb_terminal_reset_all();
        return EXIT_FAILURE;
    }

    return 0;
}
