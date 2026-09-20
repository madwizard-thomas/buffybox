/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


/*********************
 *      INCLUDES
 *********************/
#include "sq2lv_keyboard.h"
#include "sq2lv_layouts.h"

/*Testing of dependencies*/
#if LV_USE_BUTTONMATRIX == 0
    #error "lv_buttonmatrix is required. Enable it in lv_conf.h (LV_USE_BUTTONMATRIX  1) "
#endif

#if LV_USE_TEXTAREA == 0
    #error "lv_textarea is required. Enable it in lv_conf.h (LV_USE_TEXTAREA  1) "
#endif

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&sq2lv_keyboard_class)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_buttonmatrix_t btnm;
    lv_obj_t * ta;
    uint_fast8_t layout;
    uint_fast8_t layer;
    uint8_t popovers : 1;
} sq2lv_keyboard_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);

static void update_map(lv_obj_t * obj);

static bool try_to_switch_layer(lv_obj_t * kb, uint32_t btn_id);

static void on_btnm_value_changed(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t sq2lv_keyboard_class = {
    .constructor_cb = constructor,
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(50),
    .instance_size = sizeof(sq2lv_keyboard_t),
    .editable = 1,
    .base_class = &lv_buttonmatrix_class,
    .name = "sq2lv_keyboard"
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * sq2lv_keyboard_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Setter functions
 *====================*/
void sq2lv_keyboard_set_textarea(lv_obj_t * kb, lv_obj_t * ta)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    if(ta) {
        LV_ASSERT_OBJ(ta, &lv_textarea_class);
    }

    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;

    /*Hide the cursor of the old Text area if cursor management is enabled*/
    if(keyboard->ta) {
        lv_obj_remove_state(keyboard->ta, LV_STATE_FOCUSED);
    }

    keyboard->ta = ta;

    /*Show the cursor of the new Text area if cursor management is enabled*/
    if(keyboard->ta) {
        lv_obj_add_state(keyboard->ta, LV_STATE_FOCUSED);
    }
}

void sq2lv_keyboard_set_layout(lv_obj_t * kb, uint_fast8_t layout)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    if(keyboard->layout == layout) return;
    if(layout >= sq2lv_num_layouts) return;

    keyboard->layout = layout;
    keyboard->layer = 0;
    update_map(kb);
}

void sq2lv_keyboard_set_layer(lv_obj_t * kb, uint_fast8_t layer)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    if(keyboard->layer == layer) return;
    if(layer >= sq2lv_layouts[keyboard->layout].num_layers) return;

    keyboard->layer = layer;
    update_map(kb);
}

void sq2lv_keyboard_set_popovers(lv_obj_t * kb, bool en)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    if(keyboard->popovers == en) return;

    keyboard->popovers = en;

    if (keyboard->popovers)
        /* Some keys should not have popovers even if enabled, so we can't use 
           lv_buttonmatrix_set_button_ctrl_all(kb, LV_BUTTONMATRIX_CTRL_POPOVER) here.
           Instead we just set the original control map produced by squeek2lvgl. */
        lv_buttonmatrix_set_ctrl_map(kb, sq2lv_layouts[keyboard->layout].layers[keyboard->layer].attributes);
    else
        lv_buttonmatrix_clear_button_ctrl_all(kb, LV_BUTTONMATRIX_CTRL_POPOVER);
}

/*=====================
 * Getter functions
 *====================*/
lv_obj_t * sq2lv_keyboard_get_textarea(const lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    return keyboard->ta;
}

uint_fast8_t sq2lv_keyboard_get_layout(const lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    return keyboard->layout;
}

uint_fast8_t sq2lv_keyboard_get_layer(const lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    return keyboard->layer;
}

bool sq2lv_keyboard_get_popovers(const lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    return keyboard->popovers;
}

const sq2lv_layer_t * sq2lv_keyboard_get_sq2lv_layer(const lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    return &(sq2lv_layouts[keyboard->layout].layers[keyboard->layer]);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static bool try_to_switch_layer(lv_obj_t * kb, uint32_t btn_id) {
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    sq2lv_layer_t layer = sq2lv_layouts[keyboard->layout].layers[keyboard->layer];

    uint32_t n = layer.num_switchers;
    const uint32_t * sw = layer.switcher_idxs;
    for (uint32_t i = 0; i < n; ++i) {
        if (sw[i] == btn_id) {
            keyboard->layer = layer.switcher_dests[i];
            update_map(kb);
            return true;
        }
    }

    return false;
}

static void on_btnm_value_changed(lv_event_t * e)
{
    lv_obj_t * kb = lv_event_get_current_target(e);
    LV_ASSERT_OBJ(kb, MY_CLASS);
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;

    uint32_t btn_id = lv_buttonmatrix_get_selected_button(kb);
    if(btn_id == LV_BUTTONMATRIX_BUTTON_NONE) return;

    if(try_to_switch_layer(kb, btn_id))
        return;

    const char * txt = lv_buttonmatrix_get_button_text(kb, btn_id);
    if(txt == NULL) return;

    if(lv_strcmp(txt, LV_SYMBOL_OK) == 0) {
        lv_obj_send_event(kb, LV_EVENT_READY, NULL);
        return;
    }

    if(keyboard->ta == NULL) return;

    if(lv_strcmp(txt, LV_SYMBOL_LEFT) == 0) {
        lv_textarea_cursor_left(keyboard->ta);
    }
    else if(lv_strcmp(txt, LV_SYMBOL_RIGHT) == 0) {
        lv_textarea_cursor_right(keyboard->ta);
    }
    else if(lv_strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
        lv_textarea_delete_char(keyboard->ta);
    }
    else {
        lv_textarea_add_text(keyboard->ta, txt);
    }
}

static void constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) obj;
    keyboard->ta         = NULL;
    keyboard->layout     = 0;
    keyboard->layer      = 0;
    keyboard->popovers   = 1;

    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(obj, on_btnm_value_changed, LV_EVENT_VALUE_CHANGED, NULL);

    update_map(obj);
}

/**
 * Update the key and control map for the current layer
 * @param kb pointer to a keyboard object
 */
static void update_map(lv_obj_t * kb)
{
    sq2lv_keyboard_t * keyboard = (sq2lv_keyboard_t *) kb;
    sq2lv_layer_t layer = sq2lv_layouts[keyboard->layout].layers[keyboard->layer];

    lv_buttonmatrix_set_map(kb, layer.keycaps);
    lv_buttonmatrix_set_ctrl_map(kb, layer.attributes);

    if(!keyboard->popovers)
        lv_buttonmatrix_clear_button_ctrl_all(kb, LV_BUTTONMATRIX_CTRL_POPOVER);
}
