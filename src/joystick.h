/*
 * Apple // emulator for *nix
 *
 * This software package is subject to the GNU General Public License
 * version 2 or later (your choice) as published by the Free Software
 * Foundation.
 *
 * THERE ARE NO WARRANTIES WHATSOEVER.
 *
 */

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#define JOY_RANGE 0x100
#define HALF_JOY_RANGE 0x80

extern uint16_t joy_x;
extern uint16_t joy_y;
extern uint8_t joy_button0;
extern uint8_t joy_button1;
extern uint8_t joy_button2;
extern bool joy_clip_to_radius;

#ifdef KEYPAD_JOYSTICK
extern uint8_t joy_auto_recenter;
extern short joy_step;
#endif

void c_joystick_reset(void);
#ifdef INTERFACE_CLASSIC
void c_calibrate_joystick(void);
#endif

#if TOUCH_JOYSTICK

typedef enum joystick_touch_event_t {
    TOUCH_CANCEL = 0,
    TOUCH_DOWN,
    TOUCH_MOVE,
    TOUCH_UP,
    TOUCH_POINTER_DOWN,
    TOUCH_POINTER_UP,
} joystick_touch_event_t;

typedef enum touchjoy_axis_type_t {
    AXIS_EMULATED_DEVICE = 0,   // touch joystick axis emulate a physical joystick device
    AXIS_EMULATED_KEYBOARD,     // touch joystick axis send single key events
} touchjoy_axis_type_t;

// handle touch event
extern bool (*joydriver_onTouchEvent)(joystick_touch_event_t action, int pointer_count, int pointer_idx, float *x_coords, float *y_coords);

// is the touch joystick available
extern bool (*joydriver_isTouchJoystickAvailable)(void);

// enable/disable touch joystick
extern void (*joydriver_setTouchJoyEnabled)(bool enabled);

// set the joystick button parameters (7bit ASCII characters or MOUSETEXT values)
extern void (*joydriver_setTouchButtonValues)(char button0Val, char button1Val);

// set the axis type
extern void (*joydriver_setTouchAxisType)(touchjoy_axis_type_t axisType);

// set the axis button parameters (7bit ASCII characters or MOUSETEXT values)
extern void (*joydriver_setTouchAxisValues)(char up, char left, char right, char down);

#endif // TOUCH_JOYSTICK

#endif // whole file
