#ifndef ACCESSBADGE_MENU_H
#define ACCESSBADGE_MENU_H

#include <stdint.h>

typedef enum {
    MENU_WRITE = 0,
    MENU_READ = 1,
    MENU_INFOS = 2,
    MENU_COUNT = 3
} MenuOption;

void menu_display(MenuOption selected);
void clear_menu_display(void);

#endif
