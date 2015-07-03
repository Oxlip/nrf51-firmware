#ifndef __AURA_H__
#define __AURA_H__

typedef enum {
    TRIAC_OPERATION_ON,
    TRIAC_OPERATION_OFF,
    TRIAC_OPERATION_TOGGLE
} triac_operation_t;

void triac_set(int triac, triac_operation_t operation);

/** Binary format to communicate with the mobile app through BLE.
 */
typedef struct {
    uint16_t  current;  // in mA
    uint16_t  watts;
    uint8_t   volt;
    uint8_t   freq;
} __attribute__((__packed__ )) cs_info_t;

void ble_dimmer_update_value(uint16_t value);
void ble_cs_update_value(cs_info_t *cs_info);

#endif
