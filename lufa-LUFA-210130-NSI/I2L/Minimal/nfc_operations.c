#include "nfc_operations.h"
#include "pn532_i2c.h"
#include <string.h>
#include <ctype.h>

static const uint8_t KEY_DEFAULT[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static void debug_log(const char* msg) {}

bool init_pn532(PN532 *pn532) {
    uint8_t fw_buff[255];
    pn532->log = debug_log;
    PN532_I2C_Init(pn532);

    if (PN532_GetFirmwareVersion(pn532, fw_buff) != PN532_STATUS_OK)
        return false;
    return (PN532_SamConfiguration(pn532) == PN532_STATUS_OK);
}


bool helper_write_block(PN532 *pn532, uint8_t block_addr, uint8_t *uid, int32_t uid_len, 
                               uint8_t *data, const char *label) {
    // Préparation du bloc de 16 octets 
    // Remplissage 0 si data plus court
    uint8_t block_data[BLOCK_SIZE];
    memset(block_data, 0, BLOCK_SIZE);
    
    size_t len = strlen((char*)data);
    if (len > BLOCK_SIZE) len = BLOCK_SIZE;
    memcpy(block_data, data, len);

    if (PN532_MifareClassicAuthenticateBlock(pn532, uid, uid_len, block_addr, MIFARE_CMD_AUTH_A, KEY_DEFAULT) != PN532_ERROR_NONE)
        return false;

    return (PN532_MifareClassicWriteBlock(pn532, block_data, block_addr) == PN532_ERROR_NONE);
}

// Lit un bloc. Si dest_buffer != NULL, copie les données brutes.
bool helper_read_block(PN532 *pn532, uint8_t block_addr, uint8_t *uid, int32_t uid_len, 
                              uint8_t *dest_buffer, const char *label, uint8_t ui_line) {
    uint8_t raw_data[BLOCK_SIZE];
    char display_str[32];
    char safe_content[BLOCK_SIZE + 1];

    // Authentification + Lecture
    if (PN532_MifareClassicAuthenticateBlock(pn532, uid, uid_len, block_addr, MIFARE_CMD_AUTH_A, KEY_DEFAULT) == PN532_ERROR_NONE &&
        PN532_MifareClassicReadBlock(pn532, raw_data, block_addr) == PN532_ERROR_NONE) 
    {
        // Copie vers destination
        if (dest_buffer != NULL)
            memcpy(dest_buffer, raw_data, BLOCK_SIZE);

        // Nettoyage pour affichage
        memcpy(safe_content, raw_data, BLOCK_SIZE);
        safe_content[BLOCK_SIZE] = '\0';
        for (int i = 0; i < BLOCK_SIZE; i++)
            if (!isprint((unsigned char)safe_content[i])) safe_content[i] = ' ';

        snprintf(display_str, sizeof(display_str), "%s: %s", label, safe_content);
        ssd1306_print_utf8_center(display_str, ui_line);
        return true;
    } 
    else {
        snprintf(display_str, sizeof(display_str), "%s: Erreur", label);
        ssd1306_print_utf8_center(display_str, ui_line);
        return false;
    }
}
