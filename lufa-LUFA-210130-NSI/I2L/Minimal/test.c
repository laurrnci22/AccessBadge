#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

#define VENDOR_ID  0x4242
#define PRODUCT_ID 0x0001

#define EP_OUT 0x02   // MYOUT_EPADDR (OUT)
#define EP_IN  0x81   // MYIN_EPADDR  (IN)
#define EP_SIZE 8     // Taille des endpoints
#define TIMEOUT 5000  // Timeout en ms

int main(void) {
    libusb_context *ctx = NULL;
    libusb_device_handle *dev_handle = NULL;
    int r;

    // Initialisation libusb
    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "Erreur libusb_init: %d\n", r);
        return 1;
    }

    // Lister tous les périphériques USB
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(ctx, &list);
    printf("Périphériques USB détectés : %zd\n", cnt);
    for (ssize_t i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(list[i], &desc) == 0) {
            printf("VID: %04x, PID: %04x\n", desc.idVendor, desc.idProduct);
        }
    }

    // Ouvrir le périphérique LUFA
    dev_handle = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
    if (!dev_handle) {
        fprintf(stderr, "LUFA non trouvé\n");
        libusb_free_device_list(list, 1);
        libusb_exit(ctx);
        return 1;
    }
    libusb_free_device_list(list, 1);

    // Détacher driver kernel si attaché
    if (libusb_kernel_driver_active(dev_handle, 0))
        libusb_detach_kernel_driver(dev_handle, 0);

    // Claim de l'interface (bInterfaceNumber = 0 pour Minimal)
    r = libusb_claim_interface(dev_handle, 0);
    if (r != 0) {
        fprintf(stderr, "Impossible de claim l'interface: %d\n", r);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return 1;
    }

    // Envoyer un message au LUFA
    unsigned char out_msg[EP_SIZE] = "Hello!";
    int transferred;
    r = libusb_interrupt_transfer(dev_handle, EP_OUT, out_msg, EP_SIZE, &transferred, TIMEOUT);
    if (r == 0)
        printf("Envoyé %d octets\n", transferred);
    else
        printf("Erreur envoi: %d\n", r);

    // Lire une réponse du LUFA
    unsigned char in_msg[EP_SIZE] = {0};
    r = libusb_interrupt_transfer(dev_handle, EP_IN, in_msg, EP_SIZE, &transferred, TIMEOUT);
    if (r == 0)
        printf("Reçu %d octets: %.*s\n", transferred, transferred, in_msg);
    else
        printf("Erreur lecture: %d\n", r);

    // Libération et fermeture
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);

    return 0;
}
