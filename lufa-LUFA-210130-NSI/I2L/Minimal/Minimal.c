#define INCLUDE_FROM_MINIMAL_C
#include "Minimal.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <ctype.h>
#include <stdbool.h>

#include "ssd1306.h"
#include "ssd1306_text.h"
#include "pn532_i2c.h"
#include "hardware.h"
#include "menu.h"

/* ======================== CONFIGURATION & CONSTANTES ======================== */

PN532 pn532;

// Configuration USB
#define USB_BUF_SIZE    64
#define OFFSET_UID      1
#define OFFSET_NAME     8
#define OFFSET_PRENOM   24
#define OFFSET_PASS     40

// Configuration Blocs NFC
#define BLOCK_ADDR_NAME    1
#define BLOCK_ADDR_PRENOM  2
#define BLOCK_ADDR_PASS    5

// UI & Timings
#define TIMEOUT_USB_WAIT    200
#define DELAY_MSG_SHORT     500
#define DELAY_MSG_LONG      1500

/* ======================== HELPERS ======================== */

static void ui_show_msg(const char *msg, uint8_t line);
static void ui_wait_card_screen(const char* line2_text);
static void ui_format_and_print_uid(uint8_t *uid, int32_t uid_len);

/* ======================== BASES SYSTEME ======================== */

int32_t detect_card(PN532* pn532, uint8_t* uid) {
    return PN532_ReadPassiveTarget(pn532, uid, PN532_MIFARE_ISO14443A, DETECT_DELAY);
}

void usb_send_log(const char *msg) {
    Endpoint_SelectEndpoint(MYIN_EPADDR);
    if (Endpoint_IsINReady()) {
        uint8_t len = strlen(msg);
        if (len > MYIN_EPSIZE) len = MYIN_EPSIZE;
        Endpoint_Write_Stream_LE((uint8_t*)msg, len, NULL);
        Endpoint_ClearIN();
    }
}

void usb_send_response(uint8_t *buffer, uint16_t size) {
    Endpoint_SelectEndpoint(MYIN_EPADDR);
    if (Endpoint_IsINReady()) {
        Endpoint_Write_Stream_LE(buffer, size, NULL);
        Endpoint_ClearIN();
    }
}

/* ======================== ACTIONS PRINCIPALES ======================== */

void action_write(void) {
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    uint8_t usb_buffer[USB_BUF_SIZE];
    
    ssd1306_clear();
    ssd1306_print_utf8_center("En attente", 2);
    ssd1306_print_utf8_center("donnees USB...", 4);
    PORTD |= (1 << LED4);

    bool received = false;
    for (int i = 0; i < TIMEOUT_USB_WAIT; i++) {
        USB_USBTask();
        Endpoint_SelectEndpoint(MYOUT_EPADDR);
        if (Endpoint_IsOUTReceived()) {
            if (Endpoint_BytesInEndpoint() >= 52) { // Header(4) + Data(48)
                Endpoint_Read_Stream_LE(usb_buffer, USB_BUF_SIZE, NULL);
                Endpoint_ClearOUT();
                received = true;
                break;
            }
        }
        _delay_ms(50);
    }

    if (!received) {
        ui_show_msg("Timeout USB", 3);
        _delay_ms(DELAY_MSG_SHORT);
        goto cleanup;
    }

    if (usb_buffer[0] != 'W') {
        ui_show_msg("Cmd Invalide", 3);
        _delay_ms(DELAY_MSG_SHORT);
        goto cleanup;
    }

    uint8_t len_n = usb_buffer[1];
    uint8_t len_p = usb_buffer[2];
    uint8_t len_pw = usb_buffer[3];

    // Buffers à écrire max 16 octets + null 
    uint8_t data_name[17] = {0};
    uint8_t data_prenom[17] = {0};
    uint8_t data_pass[17] = {0};

    memcpy(data_name,   &usb_buffer[4],      (len_n > 16) ? 16 : len_n);
    memcpy(data_prenom, &usb_buffer[4 + 16], (len_p > 16) ? 16 : len_p);
    memcpy(data_pass,   &usb_buffer[4 + 32], (len_pw > 16) ? 16 : len_pw);

    ssd1306_clear();
    char tmp_disp[32];
    snprintf(tmp_disp, 32, "Nom: %s", data_name);
    ssd1306_print_utf8_center(tmp_disp, 1);
    snprintf(tmp_disp, 32, "Pre: %s", data_prenom);
    ssd1306_print_utf8_center(tmp_disp, 3);
    snprintf(tmp_disp, 32, "Pas: %s", data_pass);
    ssd1306_print_utf8_center(tmp_disp, 5);
    ssd1306_print_utf8_center("Approchez carte", 7);
    
    _delay_ms(DELAY_MSG_SHORT);

    int32_t uid_len = detect_card(&pn532, uid);
    if (uid_len <= 0) {
        ui_show_msg("Pas de carte", 3);
        _delay_ms(DELAY_MSG_SHORT);
        goto cleanup;
    }

    ui_show_msg("Ecriture...", 3);
    
    bool success = true;
    if (success) success = helper_write_block(&pn532, BLOCK_ADDR_NAME,   uid, uid_len, data_name,   "Nom");
    _delay_ms(20);
    if (success) success = helper_write_block(&pn532,BLOCK_ADDR_PRENOM, uid, uid_len, data_prenom, "Prenom");
    _delay_ms(20);
    if (success) success = helper_write_block(&pn532,BLOCK_ADDR_PASS,   uid, uid_len, data_pass,   "Pass");

    uint8_t response[64] = {0};
    response[0] = 'W';
    response[1] = success ? 0x01 : 0x00;
    usb_send_response(response, 64);

    ui_show_msg(success ? "Ecriture OK!" : "Echec Ecriture", 3);
    _delay_ms(DELAY_MSG_LONG);

cleanup:
    PORTD &= ~(1 << LED4);
    ssd1306_clear();
}

void action_read(void) {
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    uint8_t usb_buffer[USB_BUF_SIZE] = {0};
    
    ssd1306_clear();
    ui_wait_card_screen("pour lire...");
    PORTD |= (1 << LED5);

    int32_t uid_len = detect_card(&pn532, uid);
    if (uid_len <= 0) {
        ui_show_msg("Pas de carte", 3);
        _delay_ms(DELAY_MSG_LONG);
        goto cleanup;
    }

    ssd1306_clear();

    usb_buffer[0] = 'R'; 
    int copy_len = (uid_len > 7) ? 7 : uid_len; 
    memcpy(&usb_buffer[OFFSET_UID], uid, copy_len);

    ui_format_and_print_uid(uid, uid_len);
    
    helper_read_block(&pn532, BLOCK_ADDR_NAME,   uid, uid_len, &usb_buffer[OFFSET_NAME],   "Nom",    3);
    helper_read_block(&pn532,BLOCK_ADDR_PRENOM, uid, uid_len, &usb_buffer[OFFSET_PRENOM], "Prenom", 5);
    helper_read_block(&pn532,BLOCK_ADDR_PASS,   uid, uid_len, &usb_buffer[OFFSET_PASS],   "Pass",   7);

    usb_send_response(usb_buffer, USB_BUF_SIZE);
    
    _delay_ms(SELECTION_DELAY * 2);

cleanup:
    PORTD &= ~(1 << LED5);
    ssd1306_clear();
}


void action_infos(void) {
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    
    ssd1306_clear();
    ui_wait_card_screen("pour infos...");
    PORTD |= (1 << LED6);

    int32_t uid_len = detect_card(&pn532, uid);
    if (uid_len <= 0) {
        ui_show_msg("Pas de carte", 3);
        _delay_ms(DELAY_MSG_SHORT);
        goto cleanup;
    }

    ssd1306_clear();

    ui_format_and_print_uid(uid, uid_len);

    helper_read_block(&pn532, BLOCK_ADDR_NAME,   uid, uid_len, NULL, "Nom",    3);
    helper_read_block(&pn532, BLOCK_ADDR_PRENOM, uid, uid_len, NULL, "Prenom", 5);

    _delay_ms(SELECTION_DELAY);

cleanup:
    PORTD &= ~(1 << LED6);
    ssd1306_clear();
}

static void ui_show_msg(const char *msg, uint8_t line) {
    ssd1306_clear();
    ssd1306_print_utf8_center(msg, line);
}

static void ui_wait_card_screen(const char* line2_text) {
    ssd1306_print_utf8_center("Approchez carte", 2);
    ssd1306_print_utf8_center(line2_text, 4);
}

static void ui_format_and_print_uid(uint8_t *uid, int32_t uid_len) {
    char uid_str[32] = "UID: ";
    char tmp[4];
    for (int i = 0; i < uid_len; i++) {
        snprintf(tmp, sizeof(tmp), "%02X", uid[i]);
        strcat(uid_str, tmp);
        if (i < uid_len - 1) strcat(uid_str, " ");
    }
    ssd1306_print_utf8_center(uid_str, 1);
}


/* ======================== INITIALISATION & MAIN ======================== */

void init_system(void) {
    init_buttons();
    ssd1306_init();
    ssd1306_clear();

    ssd1306_print_utf8_center("Welcome!", 3);
    _delay_ms(WELCOME_DELAY);
    ssd1306_clear();

    ssd1306_print_utf8_center("Initialisation...", 3);

    if (!init_pn532(&pn532)) {
        ssd1306_print_utf8_center("Erreur PN532", 3);
        while (1);
    }

    USB_Init();
    GlobalInterruptEnable();
    ssd1306_clear();
}

int main(void) {
    MenuOption current_selection = MENU_WRITE;

    init_system();
    menu_display(current_selection);

    while (1) {
        USB_USBTask();

        if (button_up_pressed()) {
            current_selection = (current_selection == 0) ? MENU_COUNT - 1 : current_selection - 1;
            menu_display(current_selection);
        }

        if (button_down_pressed()) {
            current_selection = (current_selection == MENU_COUNT - 1) ? 0 : current_selection + 1;
            menu_display(current_selection);
        }

        if (button_select_pressed()) {
            switch (current_selection) {
                case MENU_WRITE: action_write(); break;
                case MENU_READ:  action_read();  break;
                case MENU_INFOS: action_infos(); break;
            }
            menu_display(current_selection);
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
