######################################################
# Altere estas variaveis de acordo com o seu setup:
#
PORT=/dev/ttyACM0
MAME=~/mame/pensebem_64
ROMPATH=~/ROM_DUMPS/FSanches/pensebem/homebrew/
ICSP_PROGRAMMER=usbasp


######################################################
#Makefile For Atmel ATMega328p (Arduino) Processor
#
#This makefile assumes single source file, includes 
#in base directory and /usr/lib/avr/include.
#
#Details:
#CC 		- avr-gcc -Cross compile using avr-gcc.
#STDLIB 	- set to include the appropriate .io file (see avr/io.h)
#LINUX_PORT	- not sure if all distros it shows up this way
#
#gcc flags:
#-std=c99 	- compile using c99
#-Wall		- enable all warnings. can also -pedantic to do strict ISO C
#-g		- produce debugging information
#-Os		- optimize for size (-O1,-O2,-O3..
#
#-Dsym		- macro defines
#-I		- include paths
#
#
MCU=atmega168pb
F_CPU=16000000UL
CC=avr-gcc
OBJ_DUMP=avr-objdump
STDLIB=__AVR_ATmega168PB__
OBJCOPY=avr-objcopy
IDIR=/usr/lib/avr/include
CFLAGS=-std=c99 -Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -D${STDLIB} -I. -I${IDIR}

TARGET=demo
SRCS=demo.c

all: bin

bin:
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SRCS}

run: bin
	${OBJCOPY} -j .text -j .data -O binary ${TARGET}.bin $(ROMPATH)/pbem2017/pensebem-2017.bin
	$(MAME) -rp $(ROMPATH) pbem2017 -window

compile:
	${CC} ${CFLAGS} -c ${SRCS}

link:
	${CC} ${CFLAGS} -o ${TARGET}.elf ${TARGET}.o

elf:
	${CC} ${CFLAGS} -c ${SRCS}
	${CC} ${CFLAGS} -o ${TARGET}.elf ${TARGET}.o
objdump:
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex
	${OBJ_DUMP} -h -S ${TARGET}.elf > ${TARGET}.lst




###############################################
#Flash processor using avrdude
#
#Options (just a few good ones):
#-F	Override invalid signature check
#-V	Do not verify
#-v	verbose output
#-y	Count number of erase cycles in eeprom
#-c	programmer type (arduino)
#-p	part number
#-P	Port
#-U	memory option specification For Example:
#
#	<memtype>:r|w|v:<filename>[:format]
#
#	Alternatives:
#	-U main.hex
#	-U flash:w:main.hex:i
#
#Either one of the following works.

flash: bin
	avrdude -F -p m168p -P $(PORT) -c $(ICSP_PROGRAMMER) -U flash:w:demo.bin

clean:
	rm -f *.bin *.hex

