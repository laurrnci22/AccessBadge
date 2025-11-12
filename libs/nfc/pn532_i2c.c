// pn532_i2c.c
#include <avr/io.h>
#include <util/delay.h>
#include "../pn532_i2c.h"

static uint8_t pn532_send_command(const uint8_t *cmd, uint8_t cmd_len);
static uint8_t pn532_read_response(uint8_t *buffer, uint8_t *length);

uint8_t pn532_init(void) {
    i2c_init();
    _delay_ms(100);

    // Configurer le SAM (Security Access Module)
    if (!pn532_sam_configuration()) {
        return 0;
    }

    return 1;
}

uint8_t pn532_wait_ready(uint16_t timeout) {
    uint8_t status;

    while (timeout--) {
        i2c_start();
        status = i2c_write((PN532_I2C_ADDRESS << 1) | 0); // Write mode

        if (status == 0x18) { // ACK received
            i2c_stop();

            // Vérifier si prêt
            i2c_start();
            i2c_write((PN532_I2C_ADDRESS << 1) | 1); // Read mode
            status = i2c_read_nack();
            i2c_stop();

            if (status == 0x01) {
                return 1;
            }
        } else {
            i2c_stop();
        }

        _delay_ms(1);
    }

    return 0;
}

uint8_t pn532_sam_configuration(void) {
    uint8_t cmd[] = {
        PN532_COMMAND_SAMCONFIGURATION,
        0x01, // Normal mode
        0x14, // Timeout 50ms * 20 = 1 seconde
        0x01  // Use IRQ pin
    };

    if (!pn532_send_command(cmd, sizeof(cmd))) {
        return 0;
    }

    uint8_t response[16];
    uint8_t len = sizeof(response);

    if (!pn532_read_response(response, &len)) {
        return 0;
    }

    return (response[0] == PN532_COMMAND_SAMCONFIGURATION + 1);
}

uint8_t pn532_read_passive_target(pn532_card_info_t *card) {
    uint8_t cmd[] = {
        PN532_COMMAND_INLISTPASSIVETARGET,
        0x01, // Max 1 carte
        PN532_MIFARE_ISO14443A
    };

    if (!pn532_send_command(cmd, sizeof(cmd))) {
        return 0;
    }

    uint8_t response[64];
    uint8_t len = sizeof(response);

    if (!pn532_read_response(response, &len)) {
        return 0;
    }

    if (response[0] != PN532_COMMAND_INLISTPASSIVETARGET + 1) {
        return 0;
    }

    uint8_t nb_targets = response[1];
    if (nb_targets != 1) {
        return 0;
    }

    // Extraire les informations de la carte
    card->uid_length = response[6];
    for (uint8_t i = 0; i < card->uid_length; i++) {
        card->uid[i] = response[7 + i];
    }

    return 1;
}

static uint8_t pn532_send_command(const uint8_t *cmd, uint8_t cmd_len) {
    uint8_t checksum = PN532_PREAMBLE + PN532_STARTCODE1 + PN532_STARTCODE2;
    uint8_t length = cmd_len + 1; // +1 pour TFI

    i2c_start();
    i2c_write((PN532_I2C_ADDRESS << 1) | 0);

    // Écrire le préambule
    i2c_write(PN532_PREAMBLE);
    i2c_write(PN532_STARTCODE1);
    i2c_write(PN532_STARTCODE2);

    // Longueur
    i2c_write(length);
    i2c_write(~length + 1); // LCS (Length checksum)

    // TFI (Frame Identifier)
    i2c_write(0xD4);
    checksum += 0xD4;

    // Commande et données
    for (uint8_t i = 0; i < cmd_len; i++) {
        i2c_write(cmd[i]);
        checksum += cmd[i];
    }

    // DCS (Data checksum)
    i2c_write(~checksum + 1);
    i2c_write(PN532_POSTAMBLE);

    i2c_stop();

    return pn532_wait_ready(1000);
}

static uint8_t pn532_read_response(uint8_t *buffer, uint8_t *length) {
    i2c_start();
    i2c_write((PN532_I2C_ADDRESS << 1) | 1);

    // Lire et ignorer le ready byte
    i2c_read_ack();

    // Vérifier préambule
    if (i2c_read_ack() != PN532_PREAMBLE ||
        i2c_read_ack() != PN532_STARTCODE1 ||
        i2c_read_ack() != PN532_STARTCODE2) {
        i2c_stop();
        return 0;
    }

    uint8_t len = i2c_read_ack();
    uint8_t lcs = i2c_read_ack();

    // Vérifier LCS
    if ((uint8_t)(len + lcs) != 0) {
        i2c_stop();
        return 0;
    }

    uint8_t tfi = i2c_read_ack();
    if (tfi != 0xD5) {
        i2c_stop();
        return 0;
    }

    len -= 1; // Soustraire TFI

    uint8_t checksum = tfi;
    for (uint8_t i = 0; i < len && i < *length; i++) {
        buffer[i] = (i < len - 1) ? i2c_read_ack() : i2c_read_nack();
        checksum += buffer[i];
    }

    *length = len;

    i2c_stop();

    return 1;
}
