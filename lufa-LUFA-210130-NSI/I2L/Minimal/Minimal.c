#define  INCLUDE_FROM_MINIMAL_C
#include "Minimal.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <ctype.h>
#include <stdbool.h>
#include <avr/io.h>

#include "ssd1306.h"
#include "ssd1306_text.h"
#include "pn532_i2c.h"

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

typedef enum {
    MENU_WRITE = 0,
    MENU_READ = 1,
    MENU_INFOS = 2,
    MENU_COUNT = 3
} MenuOption;

PN532 pn532;

static const uint8_t KEY_DEFAULT[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* ======================== BOUTONS ======================== */

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

/* ======================== AFFICHAGE MENU ======================== */

void display_menu(MenuOption selected) {
    ssd1306_clear_page(2);
    ssd1306_clear_page(4);
    ssd1306_clear_page(6);

    ssd1306_print_utf8_center(
        selected == MENU_WRITE ? "> 1. Ecrire <" : "  1. Ecrire  ", 2);

    ssd1306_print_utf8_center(
        selected == MENU_READ ? "> 2. Lire <" : "  2. Lire  ", 4);

    ssd1306_print_utf8_center(
        selected == MENU_INFOS ? "> 3. Infos <" : "  3. Infos  ", 6);
}

void show_message(const char *msg) {
    ssd1306_clear();
    ssd1306_print_utf8_center(msg, 3);
}

/* ======================== PN532 ======================== */

void debug_log(const char* msg) {}

bool write_text_to_block(PN532* pn532, uint8_t* uid, uint8_t uid_len,
                         uint8_t block_num, const char* text) {
    uint8_t data[16];
    memset(data, 0x00, sizeof(data));
    strncpy((char*)data, text, 16);

    if (PN532_MifareClassicAuthenticateBlock(pn532, uid, uid_len, block_num,
                                             MIFARE_CMD_AUTH_A, KEY_DEFAULT) != PN532_ERROR_NONE)
        return false;

    return (PN532_MifareClassicWriteBlock(pn532, data, block_num) == PN532_ERROR_NONE);
}

bool read_block(PN532* pn532, uint8_t* uid, uint8_t uid_len,
                uint8_t block_num, uint8_t* block_data) {
    if (PN532_MifareClassicAuthenticateBlock(pn532, uid, uid_len, block_num,
                                             MIFARE_CMD_AUTH_A, KEY_DEFAULT) != PN532_ERROR_NONE)
        return false;

    return (PN532_MifareClassicReadBlock(pn532, block_data, block_num) == PN532_ERROR_NONE);
}

bool init_pn532(PN532* pn532, uint8_t* fw_buff) {
    pn532->log = debug_log;
    PN532_I2C_Init(pn532);

    if (PN532_GetFirmwareVersion(pn532, fw_buff) != PN532_STATUS_OK)
        return false;

    if (PN532_SamConfiguration(pn532) != PN532_STATUS_OK)
        return false;

    return true;
}

int32_t detect_card(PN532* pn532, uint8_t* uid) {
    return PN532_ReadPassiveTarget(pn532, uid, PN532_MIFARE_ISO14443A, 1000);
}

void usb_log(const char *msg) {
    Endpoint_SelectEndpoint(MYIN_EPADDR);
    if (Endpoint_IsINReady()) {
        uint8_t len = strlen(msg);
        if (len > MYIN_EPSIZE) len = MYIN_EPSIZE;

        uint8_t buf[MYIN_EPSIZE] = {0};
        memcpy(buf, msg, len);

        Endpoint_Write_Stream_LE(buf, MYIN_EPSIZE, NULL);
        Endpoint_ClearIN();
    }
}

/* ======================== ACTIONS MENU ======================== */

void action_write(void) {
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    const uint8_t block_num = 5;
    const char* message = "Hello RFID!";

    ssd1306_clear();
    ssd1306_print_utf8_center("Approchez carte", 2);
    ssd1306_print_utf8_center("pour ecrire...", 4);

    PORTD |= (1 << LED4);

    int32_t uid_len = detect_card(&pn532, uid);
    if (uid_len <= 0) {
        show_message("Pas de carte");
        _delay_ms(1500);
        PORTD &= ~(1 << LED4);
        ssd1306_clear();
        return;
    }

    char uid_str[32] = {0};
    for (int i = 0; i < uid_len; i++) {
        char tmp[4];
        sprintf(tmp, "%02X", uid[i]);
        strcat(uid_str, tmp);
    }
    usb_log(uid_str);

    show_message("Ecriture...");

    if (write_text_to_block(&pn532, uid, uid_len, block_num, message)) {
        show_message("Ecriture OK!");
    } else {
        show_message("Erreur ecriture");
    }

    _delay_ms(SELECTION_DELAY);
    PORTD &= ~(1 << LED4);
    ssd1306_clear();
}

void action_read(void) {
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    uint8_t block_data[16];
    const uint8_t block_num = 5;

    ssd1306_clear();
    ssd1306_print_utf8_center("Approchez carte", 2);
    ssd1306_print_utf8_center("pour lire...", 4);

    PORTD |= (1 << LED5);

    int32_t uid_len = detect_card(&pn532, uid);
    if (uid_len <= 0) {
        show_message("Pas de carte");
        _delay_ms(1500);
        PORTD &= ~(1 << LED5);
        ssd1306_clear();
        return;
    }

    show_message("Lecture...");

    if (read_block(&pn532, uid, uid_len, block_num, block_data)) {
        ssd1306_clear();
        char line[32];

        sprintf(line, "Bloc %d:", block_num);
        ssd1306_print_utf8_center(line, 1);

        sprintf(line, "%02X%02X %02X%02X %02X%02X %02X%02X",
                block_data[0], block_data[1], block_data[2], block_data[3],
                block_data[4], block_data[5], block_data[6], block_data[7]);
        ssd1306_print_utf8_center(line, 3);

        sprintf(line, "%02X%02X %02X%02X %02X%02X %02X%02X",
                block_data[8], block_data[9], block_data[10], block_data[11],
                block_data[12], block_data[13], block_data[14], block_data[15]);
        ssd1306_print_utf8_center(line, 5);
    } else {
        show_message("Erreur lecture");
    }

    _delay_ms(SELECTION_DELAY);
    PORTD &= ~(1 << LED5);
    ssd1306_clear();
}

void action_infos(void) {
    uint8_t uid[MIFARE_UID_MAX_LENGTH];

    ssd1306_clear();
    ssd1306_print_utf8_center("Approchez carte", 2);
    ssd1306_print_utf8_center("pour infos...", 4);

    PORTD |= (1 << LED6);

    int32_t uid_len = detect_card(&pn532, uid);
    if (uid_len <= 0) {
        show_message("Pas de carte");
        _delay_ms(1500);
        PORTD &= ~(1 << LED6);
        ssd1306_clear();
        return;
    }

    ssd1306_clear();

    char uid_str[32] = "UID: ";
    for (int i = 0; i < uid_len; i++) {
        char tmp[4];
        sprintf(tmp, "%02X", uid[i]);
        strcat(uid_str, tmp);
        if (i < uid_len - 1) strcat(uid_str, " ");
    }
    ssd1306_print_utf8_center(uid_str, 2);

    ssd1306_print_utf8_center("Type: MIFARE", 4);
    ssd1306_print_utf8_center("Status: OK", 6);

    usb_log(uid_str);

    _delay_ms(SELECTION_DELAY);
    PORTD &= ~(1 << LED6);
    ssd1306_clear();
}

/* ======================== INITIALISATION ======================== */

void init_system(void) {
    init_buttons();
    ssd1306_init();

    show_message("Welcome!");
    _delay_ms(WELCOME_DELAY);

    show_message("Initialisation...");
    _delay_ms(INIT_DELAY);
    ssd1306_clear_page(3);

    uint8_t fw_buff[255];
    if (!init_pn532(&pn532, fw_buff)) {
        show_message("Erreur PN532");
        while (1);
    }

    USB_Init();
    GlobalInterruptEnable();
}

/* ======================== MAIN ======================== */

int main(void) {
    MenuOption current_selection = MENU_WRITE;

    init_system();

    display_menu(current_selection);

    while (1) {
        USB_USBTask();

        if (button_up_pressed()) {
            current_selection = (current_selection == 0)
                ? MENU_COUNT - 1 : current_selection - 1;
            display_menu(current_selection);
        }

        if (button_down_pressed()) {
            current_selection = (current_selection == MENU_COUNT - 1)
                ? 0 : current_selection + 1;
            display_menu(current_selection);
        }

        if (button_select_pressed()) {
            switch (current_selection) {
                case MENU_WRITE:
                    action_write();
                    break;
                case MENU_READ:
                    action_read();
                    break;
                case MENU_INFOS:
                    action_infos();
                    break;
            }
            display_menu(current_selection);
        }
    }

    return 0;
}

/* ======================== LUFA HANDLERS ======================== */

void SetupHardware(void) {
#if (ARCH == ARCH_AVR8)
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
    XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
    XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);
    XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
    XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);
    PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif
    USB_Init();
}

void EVENT_USB_Device_Connect(void) {}
void EVENT_USB_Device_Disconnect(void) {}

void EVENT_USB_Device_ConfigurationChanged(void) {
    Endpoint_ConfigureEndpoint(MYIN_EPADDR,  EP_TYPE_INTERRUPT, MYIN_EPSIZE, 1);
    Endpoint_ConfigureEndpoint(MYOUT_EPADDR, EP_TYPE_INTERRUPT, MYOUT_EPSIZE, 1);
}

void EVENT_USB_Device_ControlRequest(void) {}
