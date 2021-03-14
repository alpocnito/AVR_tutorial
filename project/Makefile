MCU = atmega8535
F_CPU = 8000000
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -std=c99 -Wall -g -O0 -mmcu=${MCU} -DF_CPU=${F_CPU} -I.
TARGET = main
SRCS = main.c

# from https://www.engbedded.com/fusecalc/
FUSE = -U lfuse:w:0xe4:m -U hfuse:w:0xd9:m

all:
	@${CC} ${CFLAGS} -o ${TARGET}.bin ${SRCS}
	@${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex

flash:
	@avrdude -p ${MCU} -c usbasp ${FUSE} -U flash:w:${TARGET}.hex:i -P usb -B24 -v

clean:
	@rm -f *.bin *.hex *.s *.o *.i
