#include <Arduino.h>
#include <HardwareSerial.h>
#include <avr/eeprom.h>
#include "ledcom.h"
#include "Streaming.h"
#include <assert.h>

/* TODO: store configuration items in EEPROM and load at run time */
#define SERIAL_BAUD      (115200) //(9600)

static void wait_for_key(void) {
	Serial.println(F("(hit any key to continue)"));
	while (!Serial && (Serial.available() == 0))
		;
}

void setup(void) {
	Serial.begin(SERIAL_BAUD);
	while (!Serial)
		; 
	Serial.println(F("loading image"));
}

void loop(void) {
	wait_for_key();
	Serial << F("\r\nstarting...");
}

