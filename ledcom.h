#ifndef LEDCOM_H
#define LEDCOM_H
#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Work out API, callback based? Blocking? Queues? */
typedef struct {
	void (*pin_configure)(void *p, int pin, int dir);
	void (*pin_set)(void *p, int pin, int on);
	int  (*pin_get)(void *p, int pin);
	unsigned (*time_us)(void *p);
	void *param;
} ledcom_t;

int ledcom_level(ledcom_t *l);

#ifdef __cplusplus
}
#endif
#endif
