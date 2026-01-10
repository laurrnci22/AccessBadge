#
#             LUFA Library
#     Copyright (C) Dean Camera, 2021.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#


MCU          = atmega32u4
ARCH         = AVR8
BOARD        = NONE
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = Minimal
export FCPU  = 16000000

# SSD1306
SSD1306_LIB_DIR = libs/libssd1306-atmega328p
SSD1306_INC_DIR = $(SSD1306_LIB_DIR)/include

#pn532 library
PN532_LIB_DIR = libs/libpn532-atmega328p
PN532_INC_DIR = $(PN532_LIB_DIR)/include

# Sources
SRC          = $(TARGET).c Descriptors.c $(LUFA_SRC_USB) hardware.c menu.c nfc_operations.c # usb_comm.c 

LUFA_PATH    = ../../LUFA


CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -DF_CPU=$(F_CPU) -IConfig/ -I$(SSD1306_INC_DIR) -I$(PN532_INC_DIR)


LD_FLAGS     = -L$(SSD1306_LIB_DIR) -lssd1306 -L$(PN532_LIB_DIR) -lpn532


all:

DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

DMBS_PATH      ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/doxygen.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk

flash: all
	sudo dfu-programmer $(MCU) erase
	sudo dfu-programmer $(MCU) flash $(TARGET).hex
	sudo dfu-programmer $(MCU) reset

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LIBUSB_CFLAGS := -I$(shell brew --prefix libusb)/include
    LIBUSB_LDFLAGS := -L$(shell brew --prefix libusb)/lib
else
    LIBUSB_CFLAGS :=
    LIBUSB_LDFLAGS :=
endif

test:
	gcc $(LIBUSB_CFLAGS) $(LIBUSB_LDFLAGS) -o lufa_comm lufa_comm.c -lusb-1.0
	sudo ./lufa_comm


