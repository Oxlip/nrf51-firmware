var assert = require('assert');
var noble = require('noble');
var async = require('async');
var nuton_uuids = require('./nuton_uuids');

var RSSI_THRESHOLD    = -90;
var EXPECTED_TIMEOUT  = 6000
var EXIT_GRACE_PERIOD = 2000; // milliseconds


/*
typedef struct {
    ble_addr    address;       // address of the device
    uint8_t     device_type;   // type of the device
    uint16_t    value;         // value to be written
} device_action_t;

// BLE message
typedef struct {
    uint8_t         add;        // 0 - update. 1 - clear
    uint8_t         button;     // button number (max 8)
    uint8_t         sub_index;  // sub action index
    device_action_t device_action;
};
*/
var  ble_msg_struct = new cstruct.Schema({
    add: cstruct.type.uint8,
    button: cstruct.type.uint8,
    sub_index: cstruct.type.uint8,

    address: cstruct.type.uint48,
    device_type: cstruct.type.uint8,
    value: cstruct.type.uint16
});

cstruct.register('ble_msg', ble_msg_struct);

describe('Reachability', function(){

    describe('Verify', function(){
        it('Verify all characteristics are present', function(done){
            this.timeout(EXPECTED_TIMEOUT);

            noble.on('discover', function(peripheral) {
                if (peripheral.rssi < RSSI_THRESHOLD) {
                    return;
                }
                if (peripheral.advertisement.localName.indexOf('Lyra') > -1) {
                    verify_characteristics(peripheral, done);
                }
            });
            noble.startScanning([], true);
        })
    })

})

/* Remove dash(-) from the given string */
function remove_dash(uuid) {
    return uuid.replace(/-/gi, '')
}

/* Verify lyra's Characterics are present */
function verify_characteristics(lyra, done) {
    var serviceUUIDs = [remove_dash(nuton_uuids.BLE_UUID_CS_SERVICE)];
    var characteristicUUIDs = [remove_dash(nuton_uuids.BLE_UUID_BUTTON_CHAR)];
    lyra.connect(function(error) {
        // set timeout
        var timeout = setTimeout(function() {
            lyra.disconnect();
            done();
        }, EXPECTED_TIMEOUT / 2);

        // discover services and characteristics
        lyra.discoverSomeServicesAndCharacteristics(serviceUUIDs, characteristicUUIDs,
            function(error, services, chars) {

                // walk through each char and write a value to it
                // and then read the value back.
                chars.forEach(function(characteristic) {
                    var buf = cstruct.packSync('ble_msg', {add:1,
                                                           button:0,
                                                           sub_index:0,
                                                           address:0x010203040506,
                                                           device_type:1,
                                                           value:100
                                                           });

                    characteristic.write(buf);

                    characteristic.read(function(error, data) {
                        console.log(data);
                    });
                });
            });
    });
}

