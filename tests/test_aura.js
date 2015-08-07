var assert = require('assert');
var noble = require('noble');
var async = require('async');
var cstruct = require('c-struct');
var nuton_uuids = require('./nuton_uuids');

var RSSI_THRESHOLD    = -90;
var EXPECTED_TIMEOUT  = 6000
var EXIT_GRACE_PERIOD = 2000; // milliseconds

var  ble_msg_struct = new cstruct.Schema({
    index: cstruct.type.uint8,
    value: cstruct.type.uint8
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
                if (peripheral.advertisement.localName.indexOf('Aura') > -1) {
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

/* Verify Aura's Characterics are present */
function verify_characteristics(aura, done) {
    var serviceUUIDs = [remove_dash(nuton_uuids.BLE_UUID_DIMMER_SERVICE),
                        remove_dash(nuton_uuids.BLE_UUID_CS_SERVICE)];
    var characteristicUUIDs = [remove_dash(nuton_uuids.BLE_UUID_DIMMER_CHAR),
                               remove_dash(nuton_uuids.BLE_UUID_CS_CHAR)];
    aura.connect(function(error) {
        // set timeout
        var timeout = setTimeout(function() {
            aura.disconnect();
            done();
        }, EXPECTED_TIMEOUT / 2);

        // discover services and characteristics
        aura.discoverSomeServicesAndCharacteristics(serviceUUIDs, characteristicUUIDs,
            function(error, services, chars) {

                // walk through each char and write a value to it
                // and then read the value back.
                chars.forEach(function(characteristic) {
                    var buf = cstruct.packSync('ble_msg', {index:2, value:50});
                    characteristic.write(buf);
                    characteristic.read(function(error, data) {
                        console.log(data);
                    });
                });
            });
    });
}

