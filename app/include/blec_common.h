/**
 * BLE related macros and functions.
 */

#ifndef __BLEC_H__
#define __BLEC_H__

#include <ble_types.h>

void blec_gap_event_timeout(const ble_gap_evt_t *p_gap_evt, uint8_t timeout_src);
void blec_gap_event_advertisement_report(ble_evt_t *p_ble_evt);
void blec_sys_event_handler(uint32_t sys_evt);
void blec_scan_start(void);
void blec_init();
void blec_on_ble_evt(ble_evt_t *p_ble_evt);

#endif
