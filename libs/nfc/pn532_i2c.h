// pn532_i2c.h
#ifndef PN532_I2C_H
#define PN532_I2C_H

#include <stdint.h>

#define PN532_I2C_ADDRESS           0x24
#define PN532_PREAMBLE              0x00
#define PN532_STARTCODE1            0x00
#define PN532_STARTCODE2            0xFF
#define PN532_POSTAMBLE             0x00

#define PN532_COMMAND_GETFIRMWAREVERSION    0x02
#define PN532_COMMAND_SAMCONFIGURATION      0x14
#define PN532_COMMAND_INLISTPASSIVETARGET   0x4A

#define PN532_MIFARE_ISO14443A      0x00

// Structures
typedef struct {
    uint8_t uid[7];
    uint8_t uid_length;
    uint8_t sak;
    uint16_t atqa;
} pn532_card_info_t;

// Fonctions I2C de base
void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);

// Fonctions PN532
uint8_t pn532_init(void);
uint8_t pn532_get_firmware_version(uint8_t *version);
uint8_t pn532_sam_configuration(void);
uint8_t pn532_read_passive_target(pn532_card_info_t *card);
uint8_t pn532_wait_ready(uint16_t timeout);

#endif
