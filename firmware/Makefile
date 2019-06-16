PRG            = main
OBJ            = main.o
MCU_TARGET     = attiny13a
OPTIMIZE       = -Os

# You should not have to change anything below here.

CC             = avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS = -g -Wall -Wl,-Map,$(PRG).map $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump
SIZE           = avr-size

all: hex

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(SIZE) -C --mcu=$(MCU_TARGET) $(PRG).elf

.PHONY: clean
clean:
	rm -rf *.o $(PRG).elf $(PRG).hex

hex:  $(PRG).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

install: load

.PHONY: load
load: $(PRG).hex
	avrdude -p t13 -c usbasp -U flash:w:$< 

