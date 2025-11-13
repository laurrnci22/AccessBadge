//
// Created by Adjy Sedar Desir on 13/11/2025.
//

#ifndef ACCESSBADGE_USB_COMM_H
#define ACCESSBADGE_USB_COMM_H

#include <stdint.h>
#include <stdbool.h>
#include "nfc_operations.h"

#define USB_BUFFER_SIZE 64
#define USB_CMD_WRITE 'W'
#define USB_CMD_READ 'R'

bool usb_receive_write_request(NFCCardData *card);
void usb_send_write_response(bool success);
void usb_send_read_data(const NFCCardData *card);


#endif //ACCESSBADGE_USB_COMM_H
