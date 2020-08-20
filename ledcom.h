#ifndef LEDCOM_H
#define LEDCOM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct {
	unsigned long tx_mark_us,
		tx_space_us,
		tx_period_us,
		rx_charge_us,
		rx_sample_us,
		rx_threshold_us;
	/*TODO: Add thresholds for mark, space, and off. */
} ledcom_sensor_t;

/* TODO: Work out API, callback based? Blocking? Queues? */
typedef struct {
	int anode, cathode, mode;
	void (*pin_configure)(void *p, int pin, int output);
	void (*pin_set)(void *p, int pin, int on);
	int  (*pin_get)(void *p, int pin);
	unsigned long (*time_us)(void *p);
	void (*sleep_us)(void *p, unsigned long us);
	void *param;
	ledcom_sensor_t const *sensor;
} ledcom_t;

int ledcom_init(ledcom_t *l);
int ledcom_level(ledcom_t *l);
int ledcom_send_octet(ledcom_t *l, unsigned char b);
int ledcom_send_octets(ledcom_t *l, const unsigned char *bs, size_t length);

#ifdef __cplusplus
}
#endif
#endif
