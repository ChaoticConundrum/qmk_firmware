#include "hal.h"

#include "quantum.h"
#include "pok3r_led.h"


typedef struct {
  ioline_t row;
  ioline_t col;
} led_matrix;

led_matrix blue_matrix[POK3R_SUPPORTED_LEDS] = {
  {LINE_ROW5, LINE_TCOL8}, // POK3R_LED_BLUE_CAPS_LOCK
  {LINE_ROW8, LINE_TCOL7}, // POK3R_LED_BLUE_SPACE_LEFT
  {LINE_ROW9, LINE_TCOL6}, // POK3R_LED_BLUE_SPACE_RIGHT
};

#define SHIFT_TROW5 0x01
#define SHIFT_TROW4 0x02
#define SHIFT_TROW3 0x04
#define SHIFT_TROW2 0x08
#define SHIFT_TROW8 0x10
#define SHIFT_TROW9 0x20
#define SHIFT_TROW7 0x40
#define SHIFT_TROW6 0x80

/* red leds are always LINE_ROW4 */
led_matrix red_matrix[POK3R_SUPPORTED_LEDS] = {
  {LINE_OD_RED3, SHIFT_TROW7}, // POK3R_LED_RED_CAPS_LOCK
  {LINE_OD_RED2, SHIFT_TROW2}, // POK3R_LED_RED_SPACE_LEFT
  {LINE_OD_RED1,  LINE_TROW1}, // POK3R_LED_RED_SPACE_RIGHT
};


static uint8_t pok3r_blue_mask;
static uint8_t pok3r_red_mask;

void pok3r_led_on(enum pok3r_led led) {
    if (pok3r_blue_mask == 0 && pok3r_red_mask == 0) {
        palClearLine(LINE_LED_VOLTAGE); // turn led voltage regulator on
    }
    if (led >= POK3R_FIRST_RED_LED) {
        pok3r_red_mask  |= (1 << (led - POK3R_FIRST_RED_LED));
    } else {
        pok3r_blue_mask |= (1 << led);
    }
}

void pok3r_led_off(enum pok3r_led led) {
    if (led >= POK3R_FIRST_RED_LED) {
        pok3r_red_mask  &= ~(1 << (led - POK3R_FIRST_RED_LED));
    } else {
        pok3r_blue_mask &= ~(1 << led);
    }
    if (pok3r_blue_mask == 0 && pok3r_red_mask == 0) {
        palSetLine(LINE_LED_VOLTAGE); // turn led voltage regulator off
    }
}

void pok3r_disable_leds(void) {
    for (int led = 0; led < POK3R_SUPPORTED_LEDS; led++) {
        /* blue leds */
        palSetLine(blue_matrix[led].row);
        palClearLine(blue_matrix[led].col);

        /* red leds*/
        palSetLine(red_matrix[led].row);
    }
    /* red leds */
    palSetLine(LINE_ROW4);
    palClearLine(LINE_LED_SHIFT_CLR);
    palClearLine(LINE_TROW1);
}

static void _prepare_shift_register(uint8_t shift_val) {
    palSetLine(LINE_LED_SHIFT_CLR);
    for (int bit = 0x80; bit > 0; bit >>= 1) {
        if (shift_val & bit)
            palSetLine(LINE_LED_SHIFT_DATA);
        else
            palClearLine(LINE_LED_SHIFT_DATA);
        palSetLine(LINE_LED_SHIFT_CLK);
        palClearLine(LINE_LED_SHIFT_CLK);
    }
}

void pok3r_enable_leds(void) {
    static uint8_t blue_or_red;
    blue_or_red = !blue_or_red;
    if (blue_or_red && pok3r_blue_mask) {
        for (int led = 0; led < POK3R_SUPPORTED_LEDS; led++) {
            if (pok3r_blue_mask & (1 << led)) {
                palSetLine(blue_matrix[led].col);
                palClearLine(blue_matrix[led].row);
            }
        }
    } else if (!blue_or_red && pok3r_red_mask) {
        uint8_t shift = 0;
        for (int led = 0; led < POK3R_SUPPORTED_LEDS; led++) {
            if (pok3r_red_mask & (1 << led)) {
                if (red_matrix[led].col != LINE_TROW1) {
                    shift |= red_matrix[led].col;
                } else {
                    palSetLine(LINE_TROW1);
                }
                palClearLine(red_matrix[led].row);
            }
        }
        _prepare_shift_register(shift);
        palClearLine(LINE_ROW4);
    }
}

void led_set_kb(uint8_t usb_led) {
    if ((usb_led >> USB_LED_CAPS_LOCK) & 1) {
        pok3r_led_on(POK3R_LED_RED_CAPS_LOCK);
    } else {
        pok3r_led_off(POK3R_LED_RED_CAPS_LOCK);
    }
    led_set_user(usb_led);
}

void pok3r_led_suspend(void) {
    if (pok3r_blue_mask != 0 || pok3r_red_mask != 0) {
        pok3r_disable_leds();
        palSetLine(LINE_LED_VOLTAGE);
    }
}

void pok3r_led_wakeup(void) {
  if (pok3r_blue_mask != 0 || pok3r_red_mask != 0) {
      palClearLine(LINE_LED_VOLTAGE);
  }
}
