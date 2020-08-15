#ifndef LEDCOM_H
#define LEDCOM_H
#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Work out API, callback based? Blocking? Queues? */
typedef struct {
	int anode, cathode, mode;
	void (*pin_configure)(void *p, int pin, int output);
	void (*pin_set)(void *p, int pin, int on);
	int  (*pin_get)(void *p, int pin);
	unsigned long (*time_us)(void *p);
	void (*sleep_us)(void *p, unsigned long us);
	void *param;
} ledcom_t;

int ledcom_init(ledcom_t *l);
int ledcom_level(ledcom_t *l);
int ledcom_send_octet(ledcom_t *l, unsigned char b);

#ifdef __cplusplus
}
#endif
#endif
