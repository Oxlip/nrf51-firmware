/*
 * Simulates Aura BLE functionality.
 *   1) Exposes the following BLE Services
 *      1. Dimmer Service
 *      2. Device Information Service(for firmware version)
 *   2) When dimmer char write happens print a message.
 *
 */

var nuton_uuids = require('./nuton_uuids');
var util = require('util');
var bleno = require('bleno');

//Dimmer Service
var DimmerCharacteristic = function() {
  DimmerCharacteristic.super_.call(this, {
    uuid: nuton_uuids.BLE_UUID_DIMMER_CHAR,
    properties: ['read', 'write', 'writeWithResponse']
  });
};

util.inherits(DimmerCharacteristic, bleno.Characteristic);

DimmerCharacteristic.prototype.onWriteRequest = function(data, offset, withoutResponse, callback) {
  console.log('DimmerCharacteristic write request: ' + data.toString('hex') + ' ' + offset + ' ' + withoutResponse);

  callback(this.RESULT_SUCCESS);
};

DimmerCharacteristic.prototype.onReadRequest = function(offset, callback) {
  var result = this.RESULT_SUCCESS;
  var data = new Buffer('dynamic value');
  console.log('DimmerCharacteristic read request: ' + offset);

  if (offset > data.length) {
    result = this.RESULT_INVALID_OFFSET;
    data = null;
  }

  callback(result, data);
};

function DimmerService() {
  DimmerService.super_.call(this, {
    uuid: nuton_uuids.BLE_UUID_DIMMER_SERVICE,
    characteristics: [
      new DimmerCharacteristic(),
    ]
  });
}

util.inherits(DimmerService, bleno.PrimaryService);

//Device Information Service

var DisCharacteristic = function() {
  DisCharacteristic.super_.call(this, {
    uuid: nuton_uuids.BLE_DIS_CHAR,
    properties: ['read'],
    value: new Buffer('1.1.0'),
    descriptors: [
      new bleno.Descriptor({
        uuid: '2901',
        value: 'Dummy descriptor'
      })
    ]
  });
};

util.inherits(DisCharacteristic, bleno.Characteristic);

function DisService() {
  DisService.super_.call(this, {
    uuid: nuton_uuids.BLE_DIS_SERVICE,
    characteristics: [
      new DisCharacteristic(),
    ]
  });
}

util.inherits(DisService, bleno.PrimaryService);


// Callbacks for BLE events.

bleno.on('stateChange', function(state) {
  console.log('on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    console.log(nuton_uuids.BLE_NUTON_UUID_AURA);
    bleno.startAdvertising('Aura', [nuton_uuids.BLE_NUTON_UUID_AURA]);
  } else {
    bleno.stopAdvertising();
  }
});

bleno.on('accept', function(clientAddress) {
  console.log('on -> accept, client: ' + clientAddress);
});

bleno.on('disconnect', function(clientAddress) {
  console.log('on -> disconnect, client: ' + clientAddress);
});


bleno.on('advertisingStart', function(error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));

  if (!error) {
    bleno.setServices([
      new DisService(),
      new DimmerService(),
    ]);
  }
});

bleno.on('advertisingStop', function() {
  console.log('on -> advertisingStop');
});
