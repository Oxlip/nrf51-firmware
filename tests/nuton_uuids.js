function define(name, value) {
    Object.defineProperty(exports, name, {
        value:      'c0f4' + value.toString(16) + '-9324-4085-aba0-0902c0e8950a',
        enumerable: true
    });
}

function define_raw_value(name, value) {
    Object.defineProperty(exports, name, {
        value:      value,
        enumerable: true
    });
}

define_raw_value("BLE_DIS_SERVICE", "0000180a-0000-1000-8000-00805f9b34fb");
define_raw_value("BLE_DIS_CHAR", "00002a26-0000-1000-8000-00805f9b34fb")

define("BLE_NUTON_UUID_AURA", 0xA000);
define("BLE_NUTON_UUID_LYRA", 0xA001);
define("BLE_NUTON_UUID_MIRA", 0xA002);

define("BLE_UUID_DIMMER_SERVICE",  0x1001);
define("BLE_UUID_CS_SERVICE",      0x1002);
define("BLE_UUID_TS_SERVICE",      0x1003);
define("BLE_UUID_HS_SERVICE",      0x1004);
define("BLE_UUID_LS_SERVICE",      0x1005);
define("BLE_UUID_MS_SERVICE",      0x1006);
define("BLE_UUID_BUTTON_SERVICE",  0x1007);

define("BLE_UUID_DIMMER_CHAR",     0x2001);
define("BLE_UUID_CS_CHAR",         0x2002);
define("BLE_UUID_TS_CHAR",         0x2003);
define("BLE_UUID_HS_CHAR",         0x2004);
define("BLE_UUID_LS_CHAR",         0x2005);
define("BLE_UUID_MS_CHAR",         0x2006);
define("BLE_UUID_BUTTON_CHAR",     0x2007);
