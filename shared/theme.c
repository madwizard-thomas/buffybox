/**
 * Copyright 2021 Johannes Marbach
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#include "theme.h"

#include "squeek2lvgl/sq2lv_keyboard.h"
#include "log.h"
#include "sq2lv_layouts.h"

/**
 * Static variables
 */

static bbx_theme current_theme;
static lv_theme_t lv_theme;

static struct {
    lv_style_t widget;
    lv_style_t window;
    lv_style_t header;
    lv_style_t keyboard;
    lv_style_t key;
#if LV_USE_BUTTON
    lv_style_t button;
    lv_style_t button_pressed;
#endif
    lv_style_t textarea;
    lv_style_t textarea_placeholder;
    lv_style_t textarea_cursor;
#if LV_USE_DROPDOWN
    lv_style_t dropdown;
    lv_style_t dropdown_pressed;
    lv_style_t dropdown_list;
    lv_style_t dropdown_list_selected;
#endif
    lv_style_t label;
#if LV_USE_MSGBOX
    lv_style_t msgbox;
    lv_style_t msgbox_label;
    lv_style_t msgbox_background;
#endif
} styles;

static bool are_styles_initialised = false;


/**
 * Static prototypes
 */

/**
 * Set up the static styles for a specific theme.
 *
 * @param theme theme to derive the styles from
 */
static void init_styles(const bbx_theme *theme);

/**
 * Initialise or reset a style.
 *
 * @param style style to reset
 */
static void reset_style(lv_style_t *style);

/**
 * Apply a theme to an object.
 *
 * @param theme theme to apply
 * @param obj object to style
 */
static void apply_theme_cb(lv_theme_t *theme, lv_obj_t *obj);

/**
 * Handle LV_EVENT_DRAW_TASK_ADDED events from the keyboard widget.
 *
 * @param event the event object
 */
static void keyboard_draw_task_added_cb(lv_event_t *event);


/**
 * Static functions
 */

static void init_styles(const bbx_theme *theme) {
    reset_style(&(styles.widget));
    lv_style_set_text_font(&(styles.widget), &bbx_font_32);

    reset_style(&(styles.window));
    lv_style_set_bg_opa(&(styles.window), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.window), lv_color_hex(theme->window.bg_color));

    reset_style(&(styles.header));
    lv_style_set_bg_opa(&(styles.header), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.header), lv_color_hex(theme->header.bg_color));
    lv_style_set_border_side(&(styles.header), LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_width(&(styles.header), lv_dpx(theme->header.border_width));
    lv_style_set_border_color(&(styles.header), lv_color_hex(theme->header.border_color));
    lv_style_set_pad_all(&(styles.header), lv_dpx(theme->header.pad));
    lv_style_set_pad_gap(&(styles.header), lv_dpx(theme->header.gap));

    reset_style(&(styles.keyboard));
    lv_style_set_bg_opa(&(styles.keyboard), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.keyboard), lv_color_hex(theme->keyboard.bg_color));
    lv_style_set_border_side(&(styles.keyboard), LV_BORDER_SIDE_TOP);
    lv_style_set_border_width(&(styles.keyboard), lv_dpx(theme->keyboard.border_width));
    lv_style_set_border_color(&(styles.keyboard), lv_color_hex(theme->keyboard.border_color));
    lv_style_set_pad_all(&(styles.keyboard), lv_dpx(theme->keyboard.pad));
    lv_style_set_pad_gap(&(styles.keyboard), lv_dpx(theme->keyboard.gap));

    reset_style(&(styles.key));
    lv_style_set_bg_opa(&(styles.key), LV_OPA_COVER);
    lv_style_set_border_side(&(styles.key), LV_BORDER_SIDE_FULL);
    lv_style_set_border_width(&(styles.key), lv_dpx(theme->keyboard.keys.border_width));
    lv_style_set_radius(&(styles.key), lv_dpx(theme->keyboard.keys.corner_radius));

#if LV_USE_BUTTON
    reset_style(&(styles.button));
    lv_style_set_text_color(&(styles.button), lv_color_hex(theme->button.normal.fg_color));
    lv_style_set_bg_opa(&(styles.button), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.button), lv_color_hex(theme->button.normal.bg_color));
    lv_style_set_border_side(&(styles.button), LV_BORDER_SIDE_FULL);
    lv_style_set_border_width(&(styles.button), lv_dpx(theme->button.border_width));
    lv_style_set_border_color(&(styles.button), lv_color_hex(theme->button.normal.border_color));
    lv_style_set_radius(&(styles.button), lv_dpx(theme->button.corner_radius));
    lv_style_set_pad_all(&(styles.button), lv_dpx(theme->button.pad));

    reset_style(&(styles.button_pressed));
    lv_style_set_text_color(&(styles.button_pressed), lv_color_hex(theme->button.pressed.fg_color));
    lv_style_set_bg_color(&(styles.button_pressed), lv_color_hex(theme->button.pressed.bg_color));
    lv_style_set_border_color(&(styles.button_pressed), lv_color_hex(theme->button.pressed.border_color));
#endif

    reset_style(&(styles.textarea));
    lv_style_set_text_color(&(styles.textarea), lv_color_hex(theme->textarea.fg_color));
    lv_style_set_bg_opa(&(styles.textarea), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.textarea), lv_color_hex(theme->textarea.bg_color));
    lv_style_set_border_side(&(styles.textarea), LV_BORDER_SIDE_FULL);
    lv_style_set_border_width(&(styles.textarea), lv_dpx(theme->textarea.border_width));
    lv_style_set_border_color(&(styles.textarea), lv_color_hex(theme->textarea.border_color));
    lv_style_set_radius(&(styles.textarea), lv_dpx(theme->textarea.corner_radius));
    lv_style_set_pad_all(&(styles.textarea), lv_dpx(theme->textarea.pad));

    reset_style(&(styles.textarea_placeholder));
    lv_style_set_text_color(&(styles.textarea_placeholder), lv_color_hex(theme->textarea.placeholder_color));

    reset_style(&(styles.textarea_cursor));
    lv_style_set_border_side(&(styles.textarea_cursor), LV_BORDER_SIDE_LEFT);
    lv_style_set_border_width(&(styles.textarea_cursor), lv_dpx(theme->textarea.cursor.width));
    lv_style_set_border_color(&(styles.textarea_cursor), lv_color_hex(theme->textarea.cursor.color));
    lv_style_set_anim_duration(&(styles.textarea_cursor), theme->textarea.cursor.period);

#if LV_USE_DROPDOWN
    reset_style(&(styles.dropdown));
    lv_style_set_text_color(&(styles.dropdown), lv_color_hex(theme->dropdown.button.normal.fg_color));
    lv_style_set_bg_opa(&(styles.dropdown), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.dropdown), lv_color_hex(theme->dropdown.button.normal.bg_color));
    lv_style_set_border_side(&(styles.dropdown), LV_BORDER_SIDE_FULL);
    lv_style_set_border_width(&(styles.dropdown), lv_dpx(theme->dropdown.button.border_width));
    lv_style_set_border_color(&(styles.dropdown), lv_color_hex(theme->dropdown.button.normal.border_color));
    lv_style_set_radius(&(styles.dropdown), lv_dpx(theme->dropdown.button.corner_radius));
    lv_style_set_pad_all(&(styles.dropdown), lv_dpx(theme->dropdown.button.pad));

    reset_style(&(styles.dropdown_pressed));
    lv_style_set_text_color(&(styles.dropdown_pressed), lv_color_hex(theme->dropdown.button.pressed.fg_color));
    lv_style_set_bg_color(&(styles.dropdown_pressed), lv_color_hex(theme->dropdown.button.pressed.bg_color));
    lv_style_set_border_color(&(styles.dropdown_pressed), lv_color_hex(theme->dropdown.button.pressed.border_color));

    reset_style(&(styles.dropdown_list));
    lv_style_set_text_color(&(styles.dropdown_list), lv_color_hex(theme->dropdown.list.fg_color));
    lv_style_set_bg_opa(&(styles.dropdown_list), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.dropdown_list), lv_color_hex(theme->dropdown.list.bg_color));
    lv_style_set_border_side(&(styles.dropdown_list), LV_BORDER_SIDE_FULL);
    lv_style_set_border_width(&(styles.dropdown_list), lv_dpx(theme->dropdown.list.border_width));
    lv_style_set_border_color(&(styles.dropdown_list), lv_color_hex(theme->dropdown.list.border_color));
    lv_style_set_radius(&(styles.dropdown_list), lv_dpx(theme->dropdown.list.corner_radius));
    lv_style_set_pad_all(&(styles.dropdown_list), lv_dpx(theme->dropdown.list.pad));

    reset_style(&(styles.dropdown_list_selected));
    lv_style_set_text_color(&(styles.dropdown_list_selected), lv_color_hex(theme->dropdown.list.selection_fg_color));
    lv_style_set_bg_opa(&(styles.dropdown_list_selected), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.dropdown_list_selected), lv_color_hex(theme->dropdown.list.selection_bg_color));
#endif

    reset_style(&(styles.label));
    lv_style_set_text_color(&(styles.label), lv_color_hex(theme->label.fg_color));

#if LV_USE_MSGBOX
    reset_style(&(styles.msgbox));
    lv_style_set_text_color(&(styles.msgbox), lv_color_hex(theme->msgbox.fg_color));
    lv_style_set_bg_opa(&(styles.msgbox), LV_OPA_COVER);
    lv_style_set_bg_color(&(styles.msgbox), lv_color_hex(theme->msgbox.bg_color));
    lv_style_set_border_side(&(styles.msgbox), LV_BORDER_SIDE_FULL);
    lv_style_set_border_width(&(styles.msgbox), lv_dpx(theme->msgbox.border_width));
    lv_style_set_border_color(&(styles.msgbox), lv_color_hex(theme->msgbox.border_color));
    lv_style_set_radius(&(styles.msgbox), lv_dpx(theme->msgbox.corner_radius));
    lv_style_set_pad_all(&(styles.msgbox), lv_dpx(theme->msgbox.pad));

    reset_style(&(styles.msgbox_label));
    lv_style_set_text_align(&(styles.msgbox_label), LV_TEXT_ALIGN_CENTER);
    lv_style_set_pad_bottom(&(styles.msgbox_label), lv_dpx(theme->msgbox.gap));

    reset_style(&(styles.msgbox_background));
    lv_style_set_bg_color(&(styles.msgbox_background), lv_color_hex(theme->msgbox.dimming.color));
    lv_style_set_bg_opa(&(styles.msgbox_background), theme->msgbox.dimming.opacity);
#endif

    are_styles_initialised = true;
}

static void reset_style(lv_style_t *style) {
    if (are_styles_initialised) {
        lv_style_reset(style);
    } else {
        lv_style_init(style);
    }
}

static void apply_theme_cb(lv_theme_t *theme, lv_obj_t *obj) {
    LV_UNUSED(theme);

    lv_obj_add_style(obj, &(styles.widget), 0);

    if (lv_obj_get_parent(obj) == NULL) {
        lv_obj_add_style(obj, &(styles.window), 0);
        return;
    }

    if (lv_obj_has_flag(obj, BBX_WIDGET_HEADER)) {
        lv_obj_add_style(obj, &(styles.header), 0);
#if LV_USE_FLEX
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
        lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);
#endif
        return;
    }

    if (lv_obj_check_type(obj, &sq2lv_keyboard_class)) {
        lv_obj_add_style(obj, &(styles.keyboard), 0);
        lv_obj_add_style(obj, &(styles.key), LV_PART_ITEMS);
        return;
    }

#if LV_USE_BUTTON
    if (lv_obj_check_type(obj, &lv_button_class)) {
        lv_obj_add_style(obj, &(styles.button), 0);
        lv_obj_add_style(obj, &(styles.button_pressed), LV_STATE_PRESSED);
        return;
    }

    if (lv_obj_check_type(obj, &lv_label_class) && lv_obj_check_type(lv_obj_get_parent(obj), &lv_button_class)) {
        return; /* Inherit styling from button */
    }
#endif

    if (lv_obj_check_type(obj, &lv_textarea_class)) {
        lv_obj_add_style(obj, &(styles.textarea), 0);
        lv_obj_add_style(obj, &(styles.textarea_placeholder), LV_PART_TEXTAREA_PLACEHOLDER);
        lv_obj_add_style(obj, &(styles.textarea_cursor), LV_PART_CURSOR | LV_STATE_FOCUSED);
        return;
    }

    if (lv_obj_check_type(obj, &lv_label_class) && lv_obj_check_type(lv_obj_get_parent(obj), &lv_textarea_class)) {
        return; /* Inherit styling from textarea */
    }

#if LV_USE_DROPDOWN
    if (lv_obj_check_type(obj, &lv_dropdown_class)) {
        lv_obj_add_style(obj, &(styles.dropdown), 0);
        lv_obj_add_style(obj, &(styles.dropdown_pressed), LV_STATE_PRESSED);
        return;
    }

    if (lv_obj_check_type(obj, &lv_dropdownlist_class)) {
        lv_obj_add_style(obj, &(styles.dropdown_list), 0);
        lv_obj_add_style(obj, &(styles.dropdown_list_selected), LV_PART_SELECTED | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &(styles.dropdown_list_selected), LV_PART_SELECTED | LV_STATE_PRESSED);
        return;
    }

    if (lv_obj_check_type(obj, &lv_label_class) && lv_obj_check_type(lv_obj_get_parent(obj), &lv_dropdownlist_class)) {
        return; /* Inherit styling from dropdown list */
    }
#endif

#if LV_USE_MSGBOX
    if (lv_obj_check_type(obj, &lv_msgbox_class)) {
        lv_obj_add_style(obj, &(styles.msgbox), 0);
        return;
    }

    if (lv_obj_check_type(obj, &lv_label_class) &&
            (lv_obj_check_type(lv_obj_get_parent(obj), &lv_msgbox_class)
             || lv_obj_check_type(lv_obj_get_parent(obj), &lv_msgbox_header_class)
             || lv_obj_check_type(lv_obj_get_parent(obj), &lv_msgbox_content_class)))
    {
        lv_obj_add_style(obj, &(styles.msgbox_label), 0);
        return; /* Inherit styling from message box */
    }

    if (lv_obj_check_type(obj, &lv_msgbox_footer_button_class)) {
        lv_obj_add_style(obj, &(styles.button), 0);
        lv_obj_add_style(obj, &(styles.button_pressed), LV_STATE_PRESSED);
        return;
    }

    if (lv_obj_check_type(obj, &lv_msgbox_backdrop_class)) {
        lv_obj_add_style(obj, &(styles.msgbox_background), 0);
        return;
    }
#endif

    if (lv_obj_check_type(obj, &lv_label_class)) {
        lv_obj_add_style(obj, &(styles.label), 0);
        return;
    }
}

static void keyboard_draw_task_added_cb(lv_event_t *event) {
    lv_obj_t *obj = lv_event_get_target(event);
    lv_buttonmatrix_t *btnm = (lv_buttonmatrix_t *)obj;
    lv_draw_task_t *draw_task = lv_event_get_draw_task(event);
    lv_draw_dsc_base_t *dsc = draw_task->draw_dsc;

    if (dsc->part != LV_PART_ITEMS) {
        return;
    }

    bbx_theme_key *key = NULL;

    if (btnm->ctrl_bits[dsc->id1] & SQ2LV_CTRL_MOD_INACTIVE) {
        key = &(current_theme.keyboard.keys.key_mod_inact);
    } else if (btnm->ctrl_bits[dsc->id1] & SQ2LV_CTRL_MOD_ACTIVE) {
        key = &(current_theme.keyboard.keys.key_mod_act);
    } else if (btnm->ctrl_bits[dsc->id1] & SQ2LV_CTRL_NON_CHAR) {
        key = &(current_theme.keyboard.keys.key_non_char);
    } else {
        key = &(current_theme.keyboard.keys.key_char);
    }

    bool pressed = lv_buttonmatrix_get_selected_button(obj) == dsc->id1 && lv_obj_has_state(obj, LV_STATE_PRESSED);

    lv_draw_label_dsc_t *label_dsc = lv_draw_task_get_label_dsc(draw_task);
    if (label_dsc) {
        label_dsc->color = lv_color_hex((pressed ? key->pressed : key->normal).fg_color);
    }

    lv_draw_fill_dsc_t *fill_dsc = lv_draw_task_get_fill_dsc(draw_task);
    if (fill_dsc) {
        fill_dsc->color = lv_color_hex((pressed ? key->pressed : key->normal).bg_color);
    }

    lv_draw_border_dsc_t *border_dsc = lv_draw_task_get_border_dsc(draw_task);
    if (border_dsc) {
        border_dsc->color = lv_color_hex((pressed ? key->pressed : key->normal).border_color);
    }
}


/**
 * Public functions
 */

void bbx_theme_prepare_keyboard(lv_obj_t *keyboard) {
    lv_obj_add_event_cb(keyboard, keyboard_draw_task_added_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
}

void bbx_theme_apply(const bbx_theme *theme) {
    if (!theme) {
        bbx_log(BBX_LOG_LEVEL_ERROR, "Could not apply theme from NULL pointer");
        return;
    }

    lv_theme.disp = NULL;
    lv_theme.font_small = &bbx_font_32;
    lv_theme.font_normal = &bbx_font_32;
    lv_theme.font_large = &bbx_font_32;
    lv_theme.apply_cb = apply_theme_cb;

    current_theme = *theme;
    init_styles(theme);

    lv_obj_report_style_change(NULL); /* Update existing objects */
    lv_display_set_theme(NULL, &lv_theme); /* Set a theme for future objects */
}
