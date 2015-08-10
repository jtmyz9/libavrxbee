NAME = xbeetest
MCU = atmega328p

DRIVER = usbtiny

CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS += -Wall -g -Os -mmcu=$(MCU)

FILES = main.c usart.c xbee.c

all:
	avr-gcc $(CFLAGS) -o $(NAME).elf $(FILES)
	avr-objcopy -O ihex $(NAME).elf $(NAME).hex

flash:
	sudo avrdude -c $(DRIVER) -p $(MCU) -U flash:w:$(NAME).hex

fuses:
	sudo avrdude -c $(DRIVER) -p $(MCU) -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m

x86:
	gcc -g -ggdb -std=c99 -o test-x86.bin x86_main.c xbee.c

clean:
	rm *.bin
	rm *.elf
	rm *.hex
