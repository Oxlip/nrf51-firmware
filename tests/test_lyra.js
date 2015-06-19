var assert = require('assert');
var noble = require('noble');
var async = require('async');
var nuton_uuids = require('./nuton_uuids');

var RSSI_THRESHOLD    = -90;
var EXPECTED_TIMEOUT  = 6000
var EXIT_GRACE_PERIOD = 2000; // milliseconds


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
                    var buf = new Buffer(2);
                    buf.writeUInt16LE(0x0010, 0);
                    characteristic.write(buf);

                    characteristic.read(function(error, data) {
                        console.log(data);
                    });
                });
            });
    });
}

