//
// Created by Adjy Sedar Desir on 13/11/2025.
//

#include "hardware.h"
#include <util/delay.h>


void init_buttons(void) {
    DDRB &= ~((1 << BTN_UP) | (1 << BTN_UP2) | (1 << BTN_DOWN2));
    PORTB |= (1 << BTN_UP) | (1 << BTN_UP2) | (1 << BTN_DOWN2);

    DDRC &= ~(1 << BTN_DOWN);
    PORTC |= (1 << BTN_DOWN);

    DDRE &= ~(1 << BTN_SELECT);
    PORTE |= (1 << BTN_SELECT);

    DDRD |= (1 << LED4) | (1 << LED5) | (1 << LED6);
}

uint8_t button_debounce(volatile uint8_t *port_reg, uint8_t pin) {
    if (!((*port_reg) & (1 << pin))) {
        _delay_ms(DEBOUNCE_MS);
        if (!((*port_reg) & (1 << pin))) {
            while (!((*port_reg) & (1 << pin)));
            _delay_ms(DEBOUNCE_MS);
            return 1;
        }
    }
    return 0;
}

uint8_t button_up_pressed(void) {
    return button_debounce(&PINB, BTN_UP) || button_debounce(&PINB, BTN_UP2);
}

uint8_t button_down_pressed(void) {
    return button_debounce(&PINC, BTN_DOWN) || button_debounce(&PINB, BTN_DOWN2);
}

uint8_t button_select_pressed(void) {
    return button_debounce(&PINE, BTN_SELECT);
}
void led_set(uint8_t led, uint8_t state) {
    if (state) {
        PORTD |= (1 << led);
    } else {
        PORTD &= ~(1 << led);
    }
}
