/**
 * Copyright 2021 Johannes Marbach
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#include "uinput_device.h"

#include "../shared/log.h"

#include <linux/uinput.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if UINPUT_VERSION < 5
#error Buffyboard does not have support for uinput < 5
#endif

/**
 * Static variables
 */

static int fd = -1;
static struct input_event events[2] = {
    { .type = EV_KEY },
    { .type = EV_SYN,
      .code = SYN_REPORT,
    }
};


/**
 * Static prototypes
 */

/**
 * Emit a key event on the device.
 * @param code event code
 * @param value event value
 * @return true if emitting the event was successful, false otherwise
 */
static bool emit_key(int code, int value);


/**
 * Static functions
 */

static bool emit_key(int code, int value) {
    events[0].code = code;
    events[0].value = value;

    if (write(fd, events, sizeof(events)) != sizeof(events)) {
        bbx_log(BBX_LOG_LEVEL_WARNING, "Could not emit events");
        return false;
    }

    return true;
}


/**
 * Public functions
 */

bool bb_uinput_device_init(const int * const keycodes, int num_keycodes) {
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        bbx_log(BBX_LOG_LEVEL_ERROR, "Could not open /dev/uinput");
        return false;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
        bbx_log(BBX_LOG_LEVEL_ERROR, "Could not set EVBIT for EV_KEY");
        return false;
    }

    for (int i = 0; i < num_keycodes; ++i) {
        if (ioctl(fd, UI_SET_KEYBIT, keycodes[i]) < 0) {
            bbx_log(BBX_LOG_LEVEL_ERROR, "Could not set KEYBIT");
            return false;
        }
    }

    static_assert(sizeof("buffyboard") <= UINPUT_MAX_NAME_SIZE);

    struct uinput_setup usetup = { 0 };
    usetup.id.bustype = BUS_VIRTUAL;
    strcpy(usetup.name, "buffyboard");

    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
        bbx_log(BBX_LOG_LEVEL_ERROR, "Could not set up uinput device");
        return false;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        bbx_log(BBX_LOG_LEVEL_ERROR, "Could not create uinput device");
        return false;
    }

    return true;
}

bool bb_uinput_device_emit_key_down(int keycode) {
    return emit_key(keycode, 1);
}

bool bb_uinput_device_emit_key_up(int keycode) {
    return emit_key(keycode, 0);
}
