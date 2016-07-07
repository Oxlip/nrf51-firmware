nrf51-firmware
==============
Firmware for nrf51822 based chips.

## Develop

### Toolchain
/usr/bin/arm-none-eabi-gcc and binutils are need to be in your system path.
gcc version(~ >= 4.8.3) with lto support is required. In Ubuntu 14.04 the
following steps would install it.

```sh
sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi
```
Ref: https://launchpad.net/~terry.guo/+archive/ubuntu/gcc-arm-embedded

### Directory Layout
    nrf51-firmware/
     --sdk/         SDK files.
     --softdevice/  Softdevice binary/hex file(s).
     --boards/      Board Specific Headers.
     --ldscripts/   Linker scripts for all boards.
     --bootloader/  Device Firmware Update.
     --common/      Common code.
     --include/     Common header files.
     --app/         App source code.
        --aura/     Aura specific files.
        --lyra/     Lyra specific files.
     --tests/       Unit tests using pytest framework.
     --tools/       Tools releated to BLE.

### Build

Since we have multiple boards(development kit, proto, production) board type has to be defined as a macro.
 BOARD_PCA10028 - Development kit(nrf51-dk).
 BOARD_AURA - Aura production/proto board.
 BOARD_LYRA - Lyra production/proto board.

If no board type is specified then PCA10028 is used.

```sh
echo "Debug version - debug symbols and no optimization"
make aura

echo "Optimized for size."
BLD_TYPE=release make aura

echo "Aura Production board and make release image"
BOARD_TYPE=AURA_V1 BLD_TYPE=release make aura

echo "Lyra Production board and make debug image"
BOARD_TYPE=LYRA_V1 make lyra

echo "Bootloader for Aura"
make aura-bootloader

echo "Bootloader for Lyra"
make lyra-bootloader

echo "Clean all object files"
make clean
```

## Flash
To flash you must have issued make command to build first and jlink installed in your system.

```sh
echo "Flash the softdevice first"
make aura-flash-softdevice
echo "Programs the app into flash"
make aura-flash

echo "Flash bootloader"
make aura-bootloader-flash

echo "Erase all contents from the flash"
make erase-all
```

### Jlink installation
Download and install JLinkExe
https://www.segger.com/jlink-software.html
