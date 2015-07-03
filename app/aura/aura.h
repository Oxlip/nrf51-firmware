#ifndef __AURA_H__
#define __AURA_H__

extern ble_ss_t dimmer_ss;
extern ble_ss_t cs_ss;

typedef enum {
    TRIAC_OPERATION_ON,
    TRIAC_OPERATION_OFF,
    TRIAC_OPERATION_TOGGLE
} triac_operation_t;
void triac_set(int triac, triac_operation_t operation);

#endif
