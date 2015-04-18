var assert = require('assert');
var noble = require('noble');

var RSSI_THRESHOLD    = -90;
var EXIT_GRACE_PERIOD = 2000; // milliseconds


describe('Reachability', function(){

    describe('Discover', function(){
        it('Check Aura is advertising', function(done){
            noble.on('discover', function(peripheral) {
                if (peripheral.rssi < RSSI_THRESHOLD) {
                    return;
                }
                if (peripheral.advertisement.localName.indexOf('Aura') > -1) {
                    done();
                }
            });
            noble.startScanning([], true);
        })
    })

    describe('Connect', function(){
        it('Connection to a advertising peripheral should happen.', function(){

        })
    })

    describe('Disconnect', function(){
        it('Disconnection from existing connection should happen immediately.', function(){

        })
    })
})
