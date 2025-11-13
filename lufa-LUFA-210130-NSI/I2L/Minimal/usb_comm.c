//
// Created by Adjy Sedar Desir on 13/11/2025.
//

#include "usb_comm.h"
#include "Descriptors.h"
#include <LUFA/Drivers/USB/USB.h>
#include <string.h>
#include <util/delay.h>

bool usb_receive_write_request(NFCCardData *card) {
    uint8_t buffer[USB_BUFFER_SIZE];

    // Attendre paquet USB (timeout 10s)
    for (int timeout = 0; timeout < 200; timeout++) {
        USB_USBTask();
        Endpoint_SelectEndpoint(MYOUT_EPADDR);

        if (Endpoint_IsOUTReceived() && Endpoint_BytesInEndpoint() >= 52) {
            Endpoint_Read_Stream_LE(buffer, USB_BUFFER_SIZE, NULL);
            Endpoint_ClearOUT();

            // Vérifier commande
            if (buffer[0] != USB_CMD_WRITE)
                return false;

            // Extraire longueurs
            uint8_t name_len = buffer[1];
            uint8_t prenom_len = buffer[2];
            uint8_t password_len = buffer[3];

            if (name_len > 16 || prenom_len > 16 || password_len > 16)
                return false;

            // Copier données
            memset(card->name, 0, sizeof(card->name));
            memset(card->prenom, 0, sizeof(card->prenom));
            memset(card->password, 0, sizeof(card->password));

            memcpy(card->name, &buffer[4], name_len);
            memcpy(card->prenom, &buffer[20], prenom_len);
            memcpy(card->password, &buffer[36], password_len);

            return true;
        }
        _delay_ms(50);
    }

    return false;
}

void usb_send_write_response(bool success) {
    uint8_t response[USB_BUFFER_SIZE] = {0};
    response[0] = USB_CMD_WRITE;
    response[1] = success ? 0x01 : 0x00;

    Endpoint_SelectEndpoint(MYIN_EPADDR);
    if (Endpoint_IsINReady()) {
        Endpoint_Write_Stream_LE(response, USB_BUFFER_SIZE, NULL);
        Endpoint_ClearIN();
    }
}

void usb_send_read_data(const NFCCardData *card) {
    uint8_t buffer[USB_BUFFER_SIZE] = {0};
    buffer[0] = USB_CMD_READ;

    // UID (octets 1-7)
    memcpy(&buffer[1], card->uid, card->uid_len);

    // Nom (octets 8-23)
    memcpy(&buffer[8], card->name, 16);

    // Prénom (octets 24-39)
    memcpy(&buffer[24], card->prenom, 16);

    // Password (octets 40-55)
    memcpy(&buffer[40], card->password, 16);

    Endpoint_SelectEndpoint(MYIN_EPADDR);
    if (Endpoint_IsINReady()) {
        Endpoint_Write_Stream_LE(buffer, USB_BUFFER_SIZE, NULL);
        Endpoint_ClearIN();
    }
}
