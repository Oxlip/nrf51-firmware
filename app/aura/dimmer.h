/**
 * Header file for the dimmer driver.
 */
#ifndef _DIMMER_H_
#define _DIMMER_H_

typedef enum {
    TRIAC_OPERATION_ON = 0,
    TRIAC_OPERATION_OFF = 1,
    TRIAC_OPERATION_TOGGLE = 2
} triac_operation_t;

void triac_set(int triac, triac_operation_t operation);

void dimmer_init(uint8_t ac_frequency);
void dimmer_enable(int triac, int percent);
void dimmer_disable(int triac);

#endif /* DIMMER_H_ */
