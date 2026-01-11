#ifndef ACCESSBADGE_NFC_OPERATIONS_H
#define ACCESSBADGE_NFC_OPERATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "pn532_i2c.h"

#define MIFARE_UID_MAX_LENGTH 7
#define BLOCK_SIZE 16
#define DETECT_DELAY 100


bool helper_write_block(PN532 *pn532, uint8_t block_addr, uint8_t *uid, int32_t uid_len, 
                               uint8_t *data, const char *label);
bool helper_read_block(PN532 *pn532, uint8_t block_addr, uint8_t *uid, int32_t uid_len, 
                              uint8_t *dest_buffer, const char *label, uint8_t ui_line);
#endif
