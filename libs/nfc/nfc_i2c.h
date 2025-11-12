// nfc_i2c.h
#ifndef NFC_I2C_H
#define NFC_I2C_H

#include <stdint.h>

// I2C configuration
#define NFC_I2C_ADDRESS 0x24  // Adresse typique pour PN532

// Fonctions I2C
void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);

// Fonctions NFC
void nfc_init(void);
uint8_t nfc_read_register(uint8_t reg);
void nfc_write_register(uint8_t reg, uint8_t value);
uint8_t nfc_read_card_uid(uint8_t *uid, uint8_t *uid_length);

#endif
