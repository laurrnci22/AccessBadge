#define  INCLUDE_FROM_MINIMAL_C
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


PN532 pn532;

static const uint8_t KEY_DEFAULT[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};



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
    return PN532_ReadPassiveTarget(pn532, uid, PN532_MIFARE_ISO14443A, DETECT_DELAY);
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
    uint8_t usb_buffer[64];
    const uint8_t name_block = 1;
    const uint8_t prenom_block = 2;
    const uint8_t password_block = 5;

    ssd1306_clear();
    ssd1306_print_utf8_center("En attente", 2);
    ssd1306_print_utf8_center("donnees USB...", 4);

    PORTD |= (1 << LED4);

    // Attendre UN SEUL paquet de 64 octets
    bool received = false;
    for (int timeout = 0; timeout < 200; timeout++) {
        USB_USBTask();
        Endpoint_SelectEndpoint(MYOUT_EPADDR);

        if (Endpoint_IsOUTReceived()) {
            uint8_t bytes_available = Endpoint_BytesInEndpoint();
            if (bytes_available >= 52) { // 4 (header) + 48 (data max)
                Endpoint_Read_Stream_LE(usb_buffer, 64, NULL);
                Endpoint_ClearOUT();
                received = true;
                break;
            }
        }
        _delay_ms(50);
    }

    if (!received) {
        show_message("Timeout USB");
        _delay_ms(500);
        PORTD &= ~(1 << LED4);
        ssd1306_clear();
        return;
    }

    // Vérifier commande 'W' (Write)
    if (usb_buffer[0] != 'W') {
        show_message("Commande invalide");
        _delay_ms(500);
        PORTD &= ~(1 << LED4);
        ssd1306_clear();
        return;
    }

    // Extraire longueurs
    uint8_t name_len = usb_buffer[1];
    uint8_t prenom_len = usb_buffer[2];
    uint8_t password_len = usb_buffer[3];

    if (name_len > 16 || prenom_len > 16 || password_len > 16) {
        show_message("Donnees trop longues");
        _delay_ms(500);
        PORTD &= ~(1 << LED4);
        ssd1306_clear();
        return;
    }

    // Préparer les blocs (données à partir de l'octet 4)
    uint8_t name_data[16], prenom_data[16], password_data[16];
    memset(name_data, 0, 16);
    memset(prenom_data, 0, 16);
    memset(password_data, 0, 16);

    memcpy(name_data, &usb_buffer[4], name_len);
    memcpy(prenom_data, &usb_buffer[4 + 16], prenom_len);
    memcpy(password_data, &usb_buffer[4 + 32], password_len);

    // Afficher les données reçues
    ssd1306_clear();
    char display[32];
    sprintf(display, "Nom: %s", name_data);
    ssd1306_print_utf8_center(display, 1);
    sprintf(display, "Prenom: %s", prenom_data);
    ssd1306_print_utf8_center(display, 3);
    sprintf(display, "Pass: %s", password_data);
    ssd1306_print_utf8_center(display, 5);
    ssd1306_print_utf8_center("Approchez carte", 7);

    _delay_ms(500);

    // Détecter la carte
    int32_t uid_len = detect_card(&pn532, uid);
    if (uid_len <= 0) {
        show_message("Pas de carte");
        _delay_ms(500);
        PORTD &= ~(1 << LED4);
        ssd1306_clear();
        return;
    }

    show_message("Ecriture...");

    bool success = true;

    // Écrire nom (bloc 1)
    if (!write_text_to_block(&pn532, uid, uid_len, name_block, (char*)name_data)) {
        success = false;
    }

    // Écrire prénom (bloc 2)
    if (success && !write_text_to_block(&pn532, uid, uid_len, prenom_block, (char*)prenom_data)) {
        success = false;
    }

    // Écrire password (bloc 5)
    if (success && !write_text_to_block(&pn532, uid, uid_len, password_block, (char*)password_data)) {
        success = false;
    }

    // Envoyer confirmation USB
    uint8_t response[64];
    memset(response, 0, 64);
    response[0] = 'W';
    response[1] = success ? 0x01 : 0x00;

    Endpoint_SelectEndpoint(MYIN_EPADDR);
    if (Endpoint_IsINReady()) {
        Endpoint_Write_Stream_LE(response, 64, NULL);
        Endpoint_ClearIN();
    }

    if (success) {
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
    const uint8_t name_block = 1;
    const uint8_t prenom_block = 2;
    const uint8_t password_block = 5;

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

    ssd1306_clear();

    // Afficher UID
    char uid_str[32] = "UID: ";
    for (int i = 0; i < uid_len; i++) {
        char tmp[4];
        sprintf(tmp, "%02X", uid[i]);
        strcat(uid_str, tmp);
        if (i < uid_len - 1) strcat(uid_str, " ");
    }
    ssd1306_print_utf8_center(uid_str, 1);

    // Préparer buffer USB (64 octets)
    uint8_t usb_buffer[64];
    memset(usb_buffer, 0, 64);
    usb_buffer[0] = 'R'; // Type: Read

    // UID (octets 1-7)
    for (int i = 0; i < uid_len && i < 7; i++) {
        usb_buffer[1 + i] = uid[i];
    }

    // Lire nom (bloc 1) -> octets 8-23
    if (read_block(&pn532, uid, uid_len, name_block, block_data)) {
        memcpy(&usb_buffer[8], block_data, 16);

        // Afficher nom
        char name[17];
        memset(name, 0, sizeof(name));
        memcpy(name, block_data, 16);
        for (int i = 0; i < 16; i++) {
            if (!isprint(name[i])) name[i] = ' ';
        }
        char name_line[32];
        sprintf(name_line, "Nom: %s", name);
        ssd1306_print_utf8_center(name_line, 3);
    } else {
        ssd1306_print_utf8_center("Nom: Erreur", 3);
    }

    // Lire prénom (bloc 2) -> octets 24-39
    if (read_block(&pn532, uid, uid_len, prenom_block, block_data)) {
        memcpy(&usb_buffer[24], block_data, 16);

        // Afficher prénom
        char prenom[17];
        memset(prenom, 0, sizeof(prenom));
        memcpy(prenom, block_data, 16);
        for (int i = 0; i < 16; i++) {
            if (!isprint(prenom[i])) prenom[i] = ' ';
        }
        char prenom_line[32];
        sprintf(prenom_line, "Prenom: %s", prenom);
        ssd1306_print_utf8_center(prenom_line, 5);
    } else {
        ssd1306_print_utf8_center("Prenom: Erreur", 5);
    }

    // Lire password (bloc 5) -> octets 40-55
    if (read_block(&pn532, uid, uid_len, password_block, block_data)) {
        memcpy(&usb_buffer[40], block_data, 16);

        // Afficher password
        char password[17];
        memset(password, 0, sizeof(password));
        memcpy(password, block_data, 16);
        for (int i = 0; i < 16; i++) {
            if (!isprint(password[i])) password[i] = ' ';
        }
        char password_line[32];
        sprintf(password_line, "Pass: %s", password);
        ssd1306_print_utf8_center(password_line, 7);
    } else {
        ssd1306_print_utf8_center("Pass: Erreur", 7);
    }

    // Envoyer via USB
    Endpoint_SelectEndpoint(MYIN_EPADDR);
    if (Endpoint_IsINReady()) {
        Endpoint_Write_Stream_LE(usb_buffer, 64, NULL);
        Endpoint_ClearIN();
    }

    _delay_ms(SELECTION_DELAY * 2); // Affichage plus long
    PORTD &= ~(1 << LED5);
    ssd1306_clear();
}



void action_infos(void) {
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    uint8_t block_data[16];
    const uint8_t name_block = 1;
    const uint8_t prenom_block = 2;

    ssd1306_clear();
    ssd1306_print_utf8_center("Approchez carte", 2);
    ssd1306_print_utf8_center("pour infos...", 4);

    PORTD |= (1 << LED6);

    int32_t uid_len = detect_card(&pn532, uid);
    if (uid_len <= 0) {
        show_message("Pas de carte");
        _delay_ms(500);
        PORTD &= ~(1 << LED6);
        ssd1306_clear();
        return;
    }

    ssd1306_clear();

    // Afficher UID
    char uid_str[32] = "UID: ";
    for (int i = 0; i < uid_len; i++) {
        char tmp[4];
        sprintf(tmp, "%02X", uid[i]);
        strcat(uid_str, tmp);
        if (i < uid_len - 1) strcat(uid_str, " ");
    }
    ssd1306_print_utf8_center(uid_str, 1);

    // Lire et afficher le nom (bloc 1)
    if (read_block(&pn532, uid, uid_len, name_block, block_data)) {
        char name[17];
        memset(name, 0, sizeof(name));
        memcpy(name, block_data, 16);
        for (int i = 0; i < 16; i++) {
            if (!isprint(name[i])) name[i] = ' ';
        }
        char name_line[32];
        sprintf(name_line, "Nom: %s", name);
        ssd1306_print_utf8_center(name_line, 3);
    } else {
        ssd1306_print_utf8_center("Nom: Erreur", 3);
    }

    // Lire et afficher le prénom (bloc 2)
    if (read_block(&pn532, uid, uid_len, prenom_block, block_data)) {
        char prenom[17];
        memset(prenom, 0, sizeof(prenom));
        memcpy(prenom, block_data, 16);
        for (int i = 0; i < 16; i++) {
            if (!isprint(prenom[i])) prenom[i] = ' ';
        }
        char prenom_line[32];
        sprintf(prenom_line, "Prenom: %s", prenom);
        ssd1306_print_utf8_center(prenom_line, 5);
    } else {
        ssd1306_print_utf8_center("Prenom: Erreur", 5);
    }

    usb_log(uid_str);

    _delay_ms(SELECTION_DELAY);
    PORTD &= ~(1 << LED6);
    ssd1306_clear();
}


/* ======================== INITIALISATION ======================== */

void init_system(void) {
    init_buttons();
    ssd1306_init();
    ssd1306_clear();

    static const uint8_t page = 3;

    ssd1306_print_utf8_center("Welcome!", page);

    _delay_ms(WELCOME_DELAY);
    ssd1306_clear_page(page);

    ssd1306_print_utf8_center("Initialisation...", page);

    uint8_t fw_buff[255];
    if (!init_pn532(&pn532, fw_buff)) {
        ssd1306_print_utf8_center("Erreur PN532", page);
        while (1);
    }

    USB_Init();
    GlobalInterruptEnable();
    ssd1306_clear_page(page);
}

/* ======================== MAIN ======================== */

int main(void) {
    MenuOption current_selection = MENU_WRITE;

    init_system();

    menu_display(current_selection);

    while (1) {
        USB_USBTask();

        if (button_up_pressed()) {
            current_selection = (current_selection == 0)
                ? MENU_COUNT - 1 : current_selection - 1;
            menu_display(current_selection);
        }

        if (button_down_pressed()) {
            current_selection = (current_selection == MENU_COUNT - 1)
                ? 0 : current_selection + 1;
            menu_display(current_selection);
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
