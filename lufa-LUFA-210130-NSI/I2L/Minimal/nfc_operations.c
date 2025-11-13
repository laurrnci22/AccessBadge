//
// Created by Adjy Sedar Desir on 13/11/2025.
//

#include "nfc_operations.h"
#include <string.h>
#include <ctype.h>

static const uint8_t KEY_DEFAULT[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static void debug_log(const char* msg) {}

bool nfc_init(PN532 *pn532) {
    uint8_t fw_buff[255];
    pn532->log = debug_log;
    PN532_I2C_Init(pn532);

    if (PN532_GetFirmwareVersion(pn532, fw_buff) != PN532_STATUS_OK)
        return false;

    return (PN532_SamConfiguration(pn532) == PN532_STATUS_OK);
}

int32_t nfc_detect_card(PN532 *pn532, uint8_t *uid) {
    return PN532_ReadPassiveTarget(pn532, uid, PN532_MIFARE_ISO14443A, DETECT_DELAY);
}

static bool read_block(PN532 *pn532, uint8_t *uid, int32_t uid_len,
                      uint8_t block_num, uint8_t *data) {
    if (PN532_MifareClassicAuthenticateBlock(pn532, uid, uid_len, block_num,
                                             MIFARE_CMD_AUTH_A, KEY_DEFAULT) != PN532_ERROR_NONE)
        return false;

    return (PN532_MifareClassicReadBlock(pn532, data, block_num) == PN532_ERROR_NONE);
}

static bool write_block(PN532 *pn532, uint8_t *uid, int32_t uid_len,
                       uint8_t block_num, const uint8_t *data) {
    if (PN532_MifareClassicAuthenticateBlock(pn532, uid, uid_len, block_num,
                                             MIFARE_CMD_AUTH_A, KEY_DEFAULT) != PN532_ERROR_NONE)
        return false;

    return (PN532_MifareClassicWriteBlock(pn532, (uint8_t*)data, block_num) == PN532_ERROR_NONE);
}

static void sanitize_text(char *dest, const uint8_t *src, size_t len) {
    memset(dest, 0, len + 1);
    memcpy(dest, src, len);
    for (size_t i = 0; i < len; i++) {
        if (!isprint(dest[i])) dest[i] = ' ';
    }
}

bool nfc_read_card(PN532 *pn532, NFCCardData *card) {
    uint8_t block_data[BLOCK_SIZE];

    // Lire nom
    if (read_block(pn532, card->uid, card->uid_len, BLOCK_NAME, block_data)) {
        sanitize_text(card->name, block_data, BLOCK_SIZE);
    } else {
        return false;
    }

    // Lire prénom
    if (read_block(pn532, card->uid, card->uid_len, BLOCK_PRENOM, block_data)) {
        sanitize_text(card->prenom, block_data, BLOCK_SIZE);
    } else {
        return false;
    }

    // Lire password
    if (read_block(pn532, card->uid, card->uid_len, BLOCK_PASSWORD, block_data)) {
        sanitize_text(card->password, block_data, BLOCK_SIZE);
    } else {
        return false;
    }

    return true;
}

bool nfc_write_card(PN532 *pn532, const NFCCardData *card) {
    uint8_t block_data[BLOCK_SIZE];

    // Écrire nom
    memset(block_data, 0, BLOCK_SIZE);
    strncpy((char*)block_data, card->name, BLOCK_SIZE);
    if (!write_block(pn532, (uint8_t*)card->uid, card->uid_len, BLOCK_NAME, block_data))
        return false;

    // Écrire prénom
    memset(block_data, 0, BLOCK_SIZE);
    strncpy((char*)block_data, card->prenom, BLOCK_SIZE);
    if (!write_block(pn532, (uint8_t*)card->uid, card->uid_len, BLOCK_PRENOM, block_data))
        return false;

    // Écrire password
    memset(block_data, 0, BLOCK_SIZE);
    strncpy((char*)block_data, card->password, BLOCK_SIZE);
    if (!write_block(pn532, (uint8_t*)card->uid, card->uid_len, BLOCK_PASSWORD, block_data))
        return false;

    return true;
}

