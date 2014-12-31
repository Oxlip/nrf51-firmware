/**
 * All UUIDs used by BLE characteristics and services.
 */

#ifndef __BLE_UUIDS__
#define __BLE_UUIDS__

#define LBS_UUID_BASE               { 0x0A, 0x95, 0xE8, 0xC0, 0x02, 0x09, 0xA0, 0xAB, 0x85, 0x40, 0x24, 0x93, 0x00, 0x00, 0xF4, 0xC0 }
#define LBS_UUID_SERVICE            0x1769
#define LBS_UUID_CMD_CHAR           0x1770
#define LBS_UUID_BUTTON_CHAR        0x1771

#define LBS_UUID_UPLUG_SERVICE      0x1001
#define LBS_UUID_UPLUG_POWER_CHAR   0x1002

#define UDEVICE_UUID_BASE { 0x0A, 0x95, 0xE8, 0xC0, 0x02, 0x09, 0xA0, 0xAB, 0x85, 0x40, 0x24, 0x93, 0x00, 0x00, 0xF4, 0xC0 }

#define UDEVICE_UUID_SERVICE     0x1000
#define UDEVICE_UUID_INFOS_CHAR  0x1001
#define UDEVICE_UUID_OUTLET_CHAR 0x1002
#define UDEVICE_UUID_SENSOR_CHAR 0x1003

#endif