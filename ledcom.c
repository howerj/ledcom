#include "ledcom.h"
#include <assert.h>

/* TODO:
 * - Using an LED as a light sensor
 * - Two way communication using an LED
 * - Change so this is non-blocking, a state-machine should
 *   control this.
 * - Add a PWM mode for emitting mode?
 *
 * This LED controller uses two I/O pins so an LED can be used
 * as both an output, and as a light sensor.
 *           
 *               Pin 1    Anode  Cathode                   Pin 2
 * Emitting:     +5v   --- (+) LED (-) ---- Resistor (1k) --  0v
 * Reverse Bias:  0V   --- (+) LED (-) ---- Resistor (1k) -- +5v
 * Discharge:     0v   --- (+) LED (-) ---- Resistor (1k) --  IN
 *
 * NB. LED cathode, negative, -, short pin
 *
 * *Circuit might need changing (input impedance too high/or input
 * floating high/too sensitive to its surroundings).  */

typedef enum {
	LED_MODE_INVALID_E,
	LED_MODE_EMIT_E,
	LED_MODE_REVERSE_BIAS_E,
	LED_MODE_DISCHARGE_E,
} led_mode_e;

enum {
	LED_PIN_INPUT_E,
	LED_PIN_OUTPUT_E,
};

typedef struct {
	unsigned long start, prev, current;
} timer_t;

typedef struct {
	unsigned long tx_mark_us,
		tx_space_us,
		tx_period_us,
		rx_charge_us,
		rx_sample_us,
		rx_threshold_us;
} led_sensor_t;

static int time_us(ledcom_t *l) { assert(l); return l->time_us(l->param); }
static void sleep_us(ledcom_t *l, unsigned long us) { assert(l); l->sleep_us(l->param, us); }

static const led_sensor_t led_settings = {  /* TODO: Read configuration from EEPROM, have serialize/deserialize functions */
	.tx_mark_us      = 1000ul,
	.tx_space_us     = 500ul,
	.tx_period_us    = 5000ul,
	.rx_charge_us    = 1000ul,
	.rx_sample_us    = 5000ul,
	.rx_threshold_us = 2500ul,
};

static int led_mode(ledcom_t *l, led_mode_e mode) {
	assert(l);
	switch (mode) {
	case LED_MODE_EMIT_E:
		l->pin_configure(l->param, l->anode,   LED_PIN_OUTPUT_E);
		l->pin_configure(l->param, l->cathode, LED_PIN_OUTPUT_E);
		l->pin_set(l->param, l->anode,   1);
		l->pin_set(l->param, l->cathode, 0);
		break;
	case LED_MODE_REVERSE_BIAS_E:
		l->pin_configure(l->param, l->anode,   LED_PIN_OUTPUT_E);
		l->pin_configure(l->param, l->cathode, LED_PIN_OUTPUT_E);
		l->pin_set(l->param, l->anode,   0);
		l->pin_set(l->param, l->cathode, 1);
		break;
	case LED_MODE_DISCHARGE_E:
		l->pin_configure(l->param, l->anode,   LED_PIN_OUTPUT_E);
		l->pin_configure(l->param, l->cathode, LED_PIN_INPUT_E);
		l->pin_set(l->param, l->anode,   0);
		break;
	default:
		return -1;
	}
	l->mode = mode;
	return 0;
}

static int led_read_cathode(ledcom_t *l) {
	assert(l);
	if (l->mode != LED_MODE_DISCHARGE_E)
		return -1;
	return l->pin_get(l->param, l->cathode);
}

static int timer_init(ledcom_t *l, timer_t *t) {
	assert(l);
	assert(t);
	t->prev    = time_us(l);
	t->start   = t->prev;
	t->current = t->prev;
	return 0;
}

static int timer_expired(ledcom_t *l, timer_t *t, const unsigned long interval_in_microseconds) {
	assert(l);
	assert(t);
	t->current = time_us(l);
	if ((t->current - t->prev) > interval_in_microseconds) {
		t->prev = t->current;
		return 1;
	}
	return 0;
}

static int led_send_bit(ledcom_t *l, int on) {
	assert(l);
	const led_sensor_t *s = &led_settings;
	unsigned long delay_us = on ? s->tx_mark_us : s->tx_space_us;
	if (led_mode(l, LED_MODE_EMIT_E) < 0)
		return -1;
	sleep_us(l, delay_us);
	if (led_mode(l, LED_MODE_REVERSE_BIAS_E) < 0)
		return -1;
	sleep_us(l, s->tx_period_us - delay_us);
	return 0;
}

int ledcom_send_octet(ledcom_t *l, unsigned char b) {
	assert(l);
	for (size_t i = 0; i < 8; i++) {
		if (led_send_bit(l, b & 1) < 0)
			return -1;
		b >>= 1;
	}
	return 0;
}

int ledcom_init(ledcom_t *l) {
	assert(l);
	return 0;
}

int ledcom_level(ledcom_t *l) {
	assert(l);
	const led_sensor_t *s = &led_settings;
	if (led_mode(l, LED_MODE_REVERSE_BIAS_E) < 0) /* charge LED */
		return -1;
	sleep_us(l, s->rx_charge_us);
	if (led_mode(l, LED_MODE_DISCHARGE_E) < 0)
		return -1;
	timer_t t;
	timer_init(l, &t);
	while (!timer_expired(l, &t, s->rx_sample_us)) {
		const int p = led_read_cathode(l);
		if (p < 0)
			return -1;
		if (p == 0)
			break;
	}
	const unsigned long r = t.current - t.start;
	sleep_us(l, s->rx_sample_us - r);
	return r > s->rx_threshold_us ? 1 : 0;
}

