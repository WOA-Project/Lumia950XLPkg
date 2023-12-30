/**
 * @file lv_btn.h
 *
 */

#ifndef LV_BTN_H
#define LV_BTN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"
#else
#include "../../lv_conf.h"
#endif

#if USE_LV_BTN != 0

/*Testing of dependencies*/
#if USE_LV_CONT == 0
#error "lv_btn: lv_cont is required. Enable it in lv_conf.h (USE_LV_CONT  1) "
#endif

#include "lv_cont.h"
#include "../lv_core/lv_indev.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/* Button states
 * It can be used not only by buttons but other button-like objects too*/
enum
{
    LV_BTN_STATE_REL,
    LV_BTN_STATE_PR,
    LV_BTN_STATE_TGL_REL,
    LV_BTN_STATE_TGL_PR,
    LV_BTN_STATE_INA,
    LV_BTN_STATE_NUM,
};
typedef uint8_t lv_btn_state_t;

enum
{
    LV_BTN_ACTION_CLICK,
    LV_BTN_ACTION_PR,
    LV_BTN_ACTION_LONG_PR,
    LV_BTN_ACTION_LONG_PR_REPEAT,
    LV_BTN_ACTION_NUM,
};
typedef uint8_t lv_btn_action_t;


/*Data of button*/
typedef struct
{
    lv_cont_ext_t cont; /*Ext. of ancestor*/
    /*New data for this type */
    lv_action_t actions[LV_BTN_ACTION_NUM];
    lv_style_t * styles[LV_BTN_STATE_NUM];        /*Styles in each state*/
    lv_btn_state_t state;                         /*Current state of the button from 'lv_btn_state_t' enum*/
#if LV_BTN_INK_EFFECT
    uint16_t ink_in_time;                         /*[ms] Time of ink fill effect (0: disable ink effect)*/
    uint16_t ink_wait_time;                       /*[ms] Wait before the ink disappears */
    uint16_t ink_out_time;                        /*[ms] Time of ink disappearing*/
#endif
    uint8_t toggle :1;                            /*1: Toggle enabled*/
    uint8_t long_pr_action_executed :1;           /*1: Long press action executed (Handled by the library)*/
} lv_btn_ext_t;

/*Styles*/
enum {
    LV_BTN_STYLE_REL,
    LV_BTN_STYLE_PR,
    LV_BTN_STYLE_TGL_REL,
    LV_BTN_STYLE_TGL_PR,
    LV_BTN_STYLE_INA,
};
typedef uint8_t lv_btn_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a button objects
 * @param par pointer to an object, it will be the parent of the new button
 * @param copy pointer to a button object, if not NULL then the new object will be copied from it
 * @return pointer to the created button
 */
lv_obj_t * lv_btn_create(lv_obj_t * par, const lv_obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Enable the toggled states. On release the button will change from/to toggled state.
 * @param btn pointer to a button object
 * @param tgl true: enable toggled states, false: disable
 */
void lv_btn_set_toggle(lv_obj_t * btn, bool tgl);

/**
 * Set the state of the button
 * @param btn pointer to a button object
 * @param state the new state of the button (from lv_btn_state_t enum)
 */
void lv_btn_set_state(lv_obj_t * btn, lv_btn_state_t state);

/**
 * Toggle the state of the button (ON->OFF, OFF->ON)
 * @param btn pointer to a button object
 */
void lv_btn_toggle(lv_obj_t * btn);

/**
 * Set a function to call when a button event happens
 * @param btn pointer to a button object
 * @param action type of event form 'lv_action_t' (press, release, long press, long press repeat)
 */
void lv_btn_set_action(lv_obj_t * btn, lv_btn_action_t type, lv_action_t action);

/**
 * Set the layout on a button
 * @param btn pointer to a button object
 * @param layout a layout from 'lv_cont_layout_t'
 */
static inline void lv_btn_set_layout(lv_obj_t * btn, lv_layout_t layout)
{
    lv_cont_set_layout(btn, layout);
}

/**
 * Enable the horizontal or vertical fit.
 * The button size will be set to involve the children horizontally or vertically.
 * @param btn pointer to a button object
 * @param hor_en true: enable the horizontal fit
 * @param ver_en true: enable the vertical fit
 */
static inline void lv_btn_set_fit(lv_obj_t * btn, bool hor_en, bool ver_en)
{
    lv_cont_set_fit(btn, hor_en, ver_en);
}

/**
 * Set time of the ink effect (draw a circle on click to animate in the new state)
 * @param btn pointer to a button object
 * @param time the time of the ink animation
 */
void lv_btn_set_ink_in_time(lv_obj_t * btn, uint16_t time);

/**
 * Set the wait time before the ink disappears
 * @param btn pointer to a button object
 * @param time the time of the ink animation
 */
void lv_btn_set_ink_wait_time(lv_obj_t * btn, uint16_t time);

/**
 * Set time of the ink out effect (animate to the released state)
 * @param btn pointer to a button object
 * @param time the time of the ink animation
 */
void lv_btn_set_ink_out_time(lv_obj_t * btn, uint16_t time);

/**
 * Set a style of a button.
 * @param btn pointer to button object
 * @param type which style should be set
 * @param style pointer to a style
 *  */
void lv_btn_set_style(lv_obj_t * btn, lv_btn_style_t type, lv_style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the current state of the button
 * @param btn pointer to a button object
 * @return the state of the button (from lv_btn_state_t enum)
 */
lv_btn_state_t lv_btn_get_state(const lv_obj_t * btn);

/**
 * Get the toggle enable attribute of the button
 * @param btn pointer to a button object
 * @return ture: toggle enabled, false: disabled
 */
bool lv_btn_get_toggle(const lv_obj_t * btn);

/**
 * Get the release action of a button
 * @param btn pointer to a button object
 * @return pointer to the release action function
 */
lv_action_t lv_btn_get_action(const lv_obj_t * btn, lv_btn_action_t type);

/**
 * Get the layout of a button
 * @param btn pointer to button object
 * @return the layout from 'lv_cont_layout_t'
 */
static inline lv_layout_t lv_btn_get_layout(const lv_obj_t * btn)
{
    return lv_cont_get_layout(btn);
}

/**
 * Get horizontal fit enable attribute of a button
 * @param btn pointer to a button object
 * @return true: horizontal fit is enabled; false: disabled
 */
static inline bool lv_btn_get_hor_fit(const lv_obj_t * btn)
{
    return lv_cont_get_hor_fit(btn);
}

/**
 * Get vertical fit enable attribute of a container
 * @param btn pointer to a button object
 * @return true: vertical fit is enabled; false: disabled
 */
static inline bool lv_btn_get_ver_fit(const lv_obj_t * btn)
{
    return lv_cont_get_ver_fit(btn);
}

/**
 * Get time of the ink in effect (draw a circle on click to animate in the new state)
 * @param btn pointer to a button object
 * @return the time of the ink animation
 */
uint16_t lv_btn_get_ink_in_time(const lv_obj_t * btn);

/**
 * Get the wait time before the ink disappears
 * @param btn pointer to a button object
 * @return the time of the ink animation
 */
uint16_t lv_btn_get_ink_wait_time(const lv_obj_t * btn);

/**
 * Get time of the ink out effect (animate to the releases state)
 * @param btn pointer to a button object
 * @return the time of the ink animation
 */
uint16_t lv_btn_get_ink_out_time(const lv_obj_t * btn);

/**
 * Get style of a button.
 * @param btn pointer to button object
 * @param type which style should be get
 * @return style pointer to the style
 *  */
lv_style_t * lv_btn_get_style(const lv_obj_t * btn, lv_btn_style_t type);

/**********************
 *      MACROS
 **********************/

#endif  /*USE_LV_BUTTON*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*LV_BTN_H*/
