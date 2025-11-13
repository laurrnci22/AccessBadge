//
// Created by Adjy Sedar Desir on 13/11/2025.
//

#ifndef ACCESSBADGE_NFC_OPERATIONS_H
#define ACCESSBADGE_NFC_OPERATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "pn532_i2c.h"

#define MIFARE_UID_MAX_LENGTH 7
#define BLOCK_SIZE 16
#define DETECT_DELAY 100

// Numéros de blocs
#define BLOCK_NAME 1
#define BLOCK_PRENOM 2
#define BLOCK_PASSWORD 5


void action_infos(void (*usb_log)(const char* msg));
#endif
