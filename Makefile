export CC = avr-gcc
export LD = avr-gcc
export MCU = atmega32u4
export FCPU = 16000000
export FLAGS = -mmcu=$(MCU)
export INCLUDES = -Ilibs/ssd1306
export CFLAGS = -Wall $(FLAGS) -DF_CPU=$(FCPU) -Os $(INCLUDES)
export LDFLAGS = $(FLAGS)
export PROGRAMMER = dfu-programmer

TARGET = main
SOURCES = $(wildcard *.c libs/ssd1306/*.c)
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET).hex

clean:
	rm -f *.o $(TARGET).hex $(TARGET)

$(TARGET): $(OBJECTS)

$(TARGET).hex: $(TARGET)
	avr-objcopy -j .text -j .data -O ihex $(TARGET) $(TARGET).hex

upload: $(TARGET).hex
	$(PROGRAMMER) $(MCU) erase
	$(PROGRAMMER) $(MCU) flash $(TARGET).hex
	$(PROGRAMMER) $(MCU) reset

size: $(TARGET)
	avr-size --format=avr --mcu=$(MCU) $(TARGET)