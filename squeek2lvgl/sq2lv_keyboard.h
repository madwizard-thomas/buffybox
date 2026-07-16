/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SQ2LV_KEYBOARD_H
#define SQ2LV_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"
#include "sq2lv_layouts.h"

#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t sq2lv_keyboard_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a Keyboard object
 * @param parent    pointer to an object, it will be the parent of the new keyboard
 * @return          pointer to the created keyboard object
 */
lv_obj_t * sq2lv_keyboard_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Assign a text area to the keyboard. Pressed characters will be inserted there.
 * @param kb        pointer to a keyboard object
 * @param ta        pointer to a text area object to write into
 */
void sq2lv_keyboard_set_textarea(lv_obj_t * kb, lv_obj_t * ta);

/**
 * Set a new layout (e.g., us, fr, de).
 * @param kb        pointer to a keyboard object
 * @param mode      the desired layout
 */
void sq2lv_keyboard_set_layout(lv_obj_t * kb, uint_fast8_t layout);

/**
 * Set a new layer (e.g., text, number, special characters).
 * @param kb        pointer to a keyboard object
 * @param mode      the desired layer
 */
void sq2lv_keyboard_set_layer(lv_obj_t * kb, uint_fast8_t layer);

/**
 * Enable or disable popovers showing button titles on press.
 * @param kb        pointer to a keyboard object
 * @param en        true to enable popovers; false to disable
 */
void sq2lv_keyboard_set_popovers(lv_obj_t * kb, bool en);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text area currently assigned to the keyboard.
 * @param kb        pointer to a keyboard object
 * @return          pointer to the assigned text area object
 */
lv_obj_t * sq2lv_keyboard_get_textarea(const lv_obj_t * kb);

/**
 * Get the current layout of the keyboard.
 * @param kb        pointer to a keyboard object
 * @return          the current layout
 */
uint_fast8_t sq2lv_keyboard_get_layout(const lv_obj_t * kb);

/**
 * Get the current layer of the keyboard.
 * @param kb        pointer to a keyboard object
 * @return          the current layer
 */
uint_fast8_t sq2lv_keyboard_get_layer(const lv_obj_t * kb);

/**
 * Check whether popovers are enabled on the keyboard.
 * @param kb        pointer to a keyboard object
 * @return          true if popovers are enabled; false otherwise
 */
bool sq2lv_keyboard_get_popovers(const lv_obj_t * kb);

/**
 * Get the currently selected sq2lv_layer_t structure
 * @param kb        pointer to a keyboard object
 * @return          the pointer to sq2lv_layer_t structure
 */
const sq2lv_layer_t * sq2lv_keyboard_get_sq2lv_layer(const lv_obj_t * kb);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*SQ2LV_KEYBOARD_H*/
