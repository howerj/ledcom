#include <Arduino.h>
#include <HardwareSerial.h>
#include <avr/eeprom.h>
#include "ledcom.h"
#include "Streaming.h"
#include <assert.h>

/* TODO: store configuration items in EEPROM and load at run time */
#define SERIAL_BAUD      (115200) //(9600)
#define UNUSED(X)        ((void)(X))

extern "C" {

static void pin_configure(void *p, int pin, int output) { UNUSED(p); pinMode(pin, output ? OUTPUT : INPUT); }
static void pin_set(void *p, int pin, int on) { UNUSED(p); digitalWrite(pin, on ? HIGH : LOW); }
static int pin_get(void *p, int pin) { UNUSED(p); return digitalRead(pin) == HIGH ? 1 : 0; }
static unsigned long time_us(void *p) { UNUSED(p); return micros(); }
static void sleep_us(void *p, unsigned long us) { UNUSED(p); delayMicroseconds(us); }

};

static ledcom_t ledcom = {
	.anode = 4, .cathode = 5, .mode = 0,
	.pin_configure = pin_configure,
	.pin_set = pin_set,
	.pin_get = pin_get,
	.time_us = time_us,
	.sleep_us = sleep_us,
	.param = NULL,
};

static void wait_for_key(void) {
	Serial.println(F("(hit any key to continue)"));
	while (!Serial && (Serial.available() == 0))
		;
}

void setup(void) {
	ledcom_init(&ledcom);
	Serial.begin(SERIAL_BAUD);
	while (!Serial)
		; 
	Serial.println(F("setup complete"));
}

void loop(void) {
	wait_for_key();
	Serial << F("\r\nstarting...");
	for(;;) {
		const int level = ledcom_level(&ledcom);
		Serial << level << F("\r\n");
	}
}

