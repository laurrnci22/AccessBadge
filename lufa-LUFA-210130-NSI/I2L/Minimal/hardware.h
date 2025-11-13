//
// Created by Adjy Sedar Desir on 13/11/2025.
//

#ifndef ACCESSBADGE_HARDWARE_H
#define ACCESSBADGE_HARDWARE_H

#include <stdint.h>
#include <avr/io.h>

#define LED4 PD4
#define LED5 PD5
#define LED6 PD6

#define BTN_UP PB4
#define BTN_UP2 PB6
#define BTN_DOWN PC6
#define BTN_DOWN2 PB5
#define BTN_SELECT PE6

#define DEBOUNCE_MS 5
#define WELCOME_DELAY 200
#define INIT_DELAY 500
#define SELECTION_DELAY 1000
#define DETECT_DELAY 100


void init_buttons(void);
uint8_t button_debounce(volatile uint8_t *port_reg, uint8_t pin);
uint8_t button_up_pressed(void);
uint8_t button_down_pressed(void) ;
uint8_t button_select_pressed(void);
void led_set(uint8_t led, uint8_t state);

#endif //ACCESSBADGE_HARDWARE_H
