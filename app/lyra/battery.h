#ifndef BATTERY_H__
#define BATTERY_H__

#include <ble_ss.h>

extern ble_ss_t battery_ss;
void battery_measure_start(void);
void battery_service_init(void);

#endif