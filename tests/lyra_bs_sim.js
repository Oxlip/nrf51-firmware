/*
 * Simulates Lyra ButtonService(non-central) BLE functionality.
 *   1) Exposes the following BLE Services
 *      1. Button Service
 *      2. Device Information Service(for firmware version)
 *   2) When button char write happens print a message.
 *
 */

var nuton_uuids = require('./nuton_uuids');
var util = require('util');
var bleno = require('bleno');

//Button Service and Characteristics
var ButtonCharacteristic = function() {
  ButtonCharacteristic.super_.call(this, {
    uuid: nuton_uuids.BLE_UUID_BUTTON_CHAR,
    properties: ['read', 'write', 'writeWithResponse']
  });
};

util.inherits(ButtonCharacteristic, bleno.Characteristic);

ButtonCharacteristic.prototype.onWriteRequest = function(data, offset, withoutResponse, callback) {
  console.log('ButtonCharacteristic write request: ' + data.toString('hex') + ' ' + offset + ' ' + withoutResponse);

  callback(this.RESULT_SUCCESS);
};

ButtonCharacteristic.prototype.onReadRequest = function(offset, callback) {
  var result = this.RESULT_SUCCESS;
  var data = new Buffer('dynamic value');
  console.log('ButtonCharacteristic read request: ' + offset);

  if (offset > data.length) {
    result = this.RESULT_INVALID_OFFSET;
    data = null;
  }

  callback(result, data);
};

function ButtonService() {
  ButtonService.super_.call(this, {
    uuid: nuton_uuids.BLE_UUID_BUTTON_SERVICE,
    characteristics: [
      new ButtonCharacteristic(),
    ]
  });
}

util.inherits(ButtonService, bleno.PrimaryService);

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
    console.log(nuton_uuids.BLE_NUTON_UUID_LYRA);
    bleno.startAdvertising('Lyra', [nuton_uuids.BLE_NUTON_UUID_LYRA]);
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
      new ButtonService(),
    ]);
  }
});

bleno.on('advertisingStop', function() {
  console.log('on -> advertisingStop');
});
