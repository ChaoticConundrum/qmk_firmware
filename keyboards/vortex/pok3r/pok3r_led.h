#ifndef POK3R_LED_H
#define POK3R_LED_H

#define POK3R_SUPPORTED_LEDS 3

enum pok3r_led {
  POK3R_LED_BLUE_CAPS_LOCK   = 0x00,
  POK3R_LED_BLUE_SPACE_LEFT  = 0x01,
  POK3R_LED_BLUE_SPACE_RIGHT = 0x02,
  // the pcb supports up to 66 LEDs
  POK3R_LED_RED_CAPS_LOCK    = 0x42,
  POK3R_LED_RED_SPACE_LEFT   = 0x43,
  POK3R_LED_RED_SPACE_RIGHT  = 0x44,
};
#define POK3R_FIRST_RED_LED POK3R_LED_RED_CAPS_LOCK

void pok3r_led_on(enum pok3r_led led);
void pok3r_led_off(enum pok3r_led led);

#endif
