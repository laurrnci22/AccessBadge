#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <libusb-1.0/libusb.h>

#define VENDOR_ID  0x4242
#define PRODUCT_ID 0x0001

#define EP_OUT 0x02   // MYOUT_EPADDR (OUT)
#define EP_IN  0x81   // MYIN_EPADDR  (IN)
#define EP_SIZE 8     // Taille des endpoints
#define TIMEOUT 100  // Timeout en ms
#define TIMEOUT 100  // Timeout en ms

int main(void)
{
    libusb_context *ctx = NULL;
    libusb_device_handle *dev_handle = NULL;
    int r;

    // Initialisation libusb
    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "Erreur libusb_init: %d\n", r);
        return 1;
    }

    // Ouvrir le périphérique LUFA
    dev_handle = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
    if (!dev_handle) {
        fprintf(stderr, "LUFA non trouvé\n");
        libusb_exit(ctx);
        return 1;
    }

    // Détacher driver kernel si attaché
    if (libusb_kernel_driver_active(dev_handle, 0))
        libusb_detach_kernel_driver(dev_handle, 0);

    // Claim de l'interface
    r = libusb_claim_interface(dev_handle, 0);
    if (r != 0) {
        fprintf(stderr, "Impossible de claim l'interface: %d\n", r);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return 1;
    }

    printf("=== Monitoring USB ===\n");
    printf("Appuyez sur Ctrl+C pour arrêter\n\n");

    // Boucle de monitoring continu
    while (1) {
        unsigned char in_msg[EP_SIZE] = {0};
        int transferred;
        r = libusb_interrupt_transfer(dev_handle, EP_IN, in_msg, EP_SIZE, &transferred, TIMEOUT);

        if (r == 0 && transferred > 0) {
            printf("[%ld] %.*s\n", time(NULL), transferred, in_msg);
            fflush(stdout);  // Force l'affichage immédiat
        }

        usleep(10000); // 10ms entre chaque lecture
    }

    // Libération (jamais atteint sauf si Ctrl+C)
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);

    return 0;
}

