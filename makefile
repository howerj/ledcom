# Simple Arduino Makefile
#
# See:
# https://arduino.stackexchange.com/questions/15893

F_CPU = 16000000
MCU = atmega328p
#MCU = atmega2560
# 'arduino' for atmega328p, 'wiring' for atmega2560
METHOD=arduino
#METHOD=wiring
PORT=/dev/ttyACM0
PROGRAM_BAUD=115200
BAUD=115200

ARDUINO_DIR = /usr/share/arduino/
CC      = ${ARDUINO_DIR}hardware/tools/avr/bin/avr-gcc
CPP     = ${ARDUINO_DIR}hardware/tools/avr/bin/avr-g++
AR      = ${ARDUINO_DIR}hardware/tools/avr/bin/avr-ar
OBJCOPY = ${ARDUINO_DIR}hardware/tools/avr/bin/avr-objcopy

CORE_CSRC= \
 ${LIBRARY_DIR}avr-libc/malloc.c \
 ${LIBRARY_DIR}avr-libc/realloc.c \
 ${LIBRARY_DIR}WInterrupts.c \
 ${LIBRARY_DIR}wiring.c \
 ${LIBRARY_DIR}wiring_analog.c \
 ${LIBRARY_DIR}wiring_digital.c \
 ${LIBRARY_DIR}wiring_pulse.c \
 ${LIBRARY_DIR}wiring_shift.c

CORE_CPPSRC= \
 ${LIBRARY_DIR}CDC.cpp \
 ${LIBRARY_DIR}HardwareSerial.cpp \
 ${LIBRARY_DIR}HID.cpp \
 ${LIBRARY_DIR}IPAddress.cpp \
 ${LIBRARY_DIR}main.cpp \
 ${LIBRARY_DIR}new.cpp \
 ${LIBRARY_DIR}Print.cpp \
 ${LIBRARY_DIR}Stream.cpp \
 ${LIBRARY_DIR}Tone.cpp \
 ${LIBRARY_DIR}USBCore.cpp \
 ${LIBRARY_DIR}WMath.cpp \
 ${LIBRARY_DIR}WString.cpp 

.PRECIOUS: %.o

CORE_COBJS   := ${CORE_CSRC:%.c=%.o}
CORE_CXXOBJS := ${CORE_CPPSRC:%.cpp=%.o}

CORE_OBJS    := ${CORE_COBJS} ${CORE_CXXOBJS}
CORE_OBJS    := ${CORE_OBJS:${LIBRARY_DIR}%=%}
CORE_OBJS    := ${CORE_OBJS:avr-libc/%=%}

TARGET=test
CSRC := ${TARGET}.cpp ledcom.c
OBJS := ${CSRC:%.c=%.o}
OBJS := ${OBJS:%.cpp=%.o}

.PHONY: all build mkdebug upload talk

all: build

CPPFLAGS := -c -g -Os -Wall -Wextra -ffunction-sections -fdata-sections -mmcu=${MCU} -DF_CPU=${F_CPU}L -DUSB_VID=null -DUSB_PID=null -DARDUINO=106 
CPPFLAGS := ${CPPFLAGS} -DNDEBUG
CXXFLAGS := ${CPPFLAGS} -fno-exceptions
CFLAGS   := ${CPPFLAGS} -std=gnu99

INCLUDE_FILES = -I${ARDUINO_DIR}hardware/arduino/cores/arduino -I${ARDUINO_DIR}hardware/arduino/variants/standard
LIBRARY_DIR   = ${ARDUINO_DIR}hardware/arduino/cores/arduino/

upload: ${TARGET}.hex
	avrdude -C ${ARDUINO_DIR}hardware/tools/avrdude.conf -p ${MCU} -c ${METHOD} -P ${PORT} -b ${PROGRAM_BAUD} -D -Uflash:w:$^:i 

%.o: %.cpp
	${CPP} ${CXXFLAGS} ${INCLUDE_FILES} $< -o $@

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDE_FILES} $< -o $@

%.o: ${LIBRARY_DIR}%.cpp
	${CPP} ${CXXFLAGS} ${INCLUDE_FILES} $< -o $@

%.o: ${LIBRARY_DIR}%.c
	${CC} ${CFLAGS} ${INCLUDE_FILES} $< -o $@

%.o: ${LIBRARY_DIR}/avr-libc/%.c
	${CC} ${CFLAGS} ${INCLUDE_FILES} $< -o $@

core.a: ${CORE_OBJS}
	rm -fv $@
	${AR} rcs $@ $^

%.elf: ${OBJS} core.a
	${CC} -Os -Wl,--gc-sections -mmcu=${MCU} -o $@ $^ -lm

%.eep: %.elf
	${OBJCOPY} -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< $@

%.hex: %.elf %.eep
	${OBJCOPY} -O ihex -R .eeprom $< $@

%.bin: %.hex
	${OBJCOPY} -I ihex $< -O binary $@

build: ${TARGET}.hex ${TARGET}.bin

mkdebug:
	@echo ${CORE_OBJS}

talk:
	picocom -e b -b ${BAUD} ${PORT}

clean:
	rm -vf *.o *.a *.d *.elf *.eep *.hex *.d

