//
// Created by Adjy Sedar Desir on 13/11/2025.
//

#include "menu.h"

#include <stdbool.h>

#include "ssd1306.h"
#include "ssd1306_text.h"
#include <stdio.h>
#include <string.h>

static const uint8_t MENU_START_PAGE = 2;
static const uint8_t MENU_PAGE_SPACING = 2;

static const char* MENU_LABELS[] = {
    "Ecrire",
    "Lire",
    "Infos"
};

static const uint8_t MENU_ITEM_COUNT = sizeof(MENU_LABELS) / sizeof(MENU_LABELS[0]);

static uint8_t get_page_for_item(uint8_t item_index) {
    return MENU_START_PAGE + (item_index * MENU_PAGE_SPACING);
}

static void format_menu_line(char* buffer, uint8_t item_index, bool is_selected) {
    if (is_selected) {
        sprintf(buffer, "> %d. %s <", item_index + 1, MENU_LABELS[item_index]);
    } else {
        sprintf(buffer, "  %d. %s  ", item_index + 1, MENU_LABELS[item_index]);
    }
}

void menu_display(MenuOption selected) {
    char line_buffer[32];


    for (uint8_t i = 0; i < MENU_ITEM_COUNT; i++) {
        uint8_t page = get_page_for_item(i);

        format_menu_line(line_buffer, i, (selected == i));
        ssd1306_print_utf8_center(line_buffer, page);
    }
}

void clear_menu_display(void) {
    for (uint8_t i = 0; i < MENU_ITEM_COUNT; i++) {
        uint8_t page = get_page_for_item(i);
        ssd1306_clear_page(page);
    }
}
