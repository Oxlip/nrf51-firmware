var nuton_uuids = require('./nuton_uuids');
var util = require('util');
var bleno = require('bleno');


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
      new DimmerService()
    ]);
  }
});

bleno.on('advertisingStop', function() {
  console.log('on -> advertisingStop');
});

bleno.on('servicesSet', function() {
  console.log('on -> servicesSet');
});


