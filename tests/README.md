# Tests


Functionality tests(mainly BLE) of Aura, Lyra and Mira.

# Requirement

1. Bluetooth(4.0 or >). (Tested with Pluggable USB BT 4.0 dongle)
1. Aura/Lyra or nrf51 devboard.
1. Linux (Tested with Ubuntu 14.04 VM)

BLE library(node.js) used:
https://github.com/sandeepmistry/noble (central)
https://github.com/sandeepmistry/bleno (peripheral)

Since these libraries are based on node.js, the testing framework used
is mocha(http://mochajs.org/)

## Install

### nodejs and mocha
```
sudo apt-get update
sudo apt-get install node nodejs npm
sudo npm install -g mocha
sudo ln -s /usr/bin/nodejs /usr/bin/node
```
### noble and bleno
```
sudo apt-get install bluetooth bluez-utils libbluetooth-dev
cd <repo>/tests
sudo npm install noble bleno c-struct
sudo apt-get install libcap2-bin
find -path '*noble*Release/hci-ble' -exec sudo setcap cap_net_raw+eip '{}' \;
```

# Running
To start tests
```
cd <repo>
sudo mocha tests
```

To start simulator
```
cd <repo>/tests
sudo nodejs ./aura_sim.js
```


Note: if there is an error message try resetting the bt dongle.
```
sudo rfkill unblock bluetooth
sudo hciconfig hci0 down
sudo hciconfig hci0 up
```
