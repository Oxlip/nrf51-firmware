nrf51-firmware
==============
Firmware for nrf51822 based chips.

## Develop
/usr/bin/arm-none-eabi-gcc and binutils are need to be in your system path.
gcc version(~ >= 4.8.3) with lto support is required.

### Directory Layout
    nrf51-firmware/
     --toolchain/   Toolchain setup.
     --sdk/         SDK headers.
     --lib/         Library binary/hex file(s).
     --tools/       Tools releated to BLE.
     --tests/       Unit tests using pytest framework.
     --bootloader/  Device Firmware Update.
     --app/         App source code.
        platform.c  Common nrf51 related code.
        ble.c       Common source code related to BLE.
        6lowpan.c   Common code related to 6lowpan.
        Makefile    Common defines
        --include/  Common header files.
        --aura/     Aura specific files.
        --lyra/     Lyra specific files.
        --mira/     Mira specific files.

### Build

To build image go to the appropriate device directory(aura, lyra etc) and issue *make* command.
Since we have multiple boards(development kit, proto, production) board type has to be defined as a macro.
 BOARD_AURA - Aura production/proto board. 
 BOARD_DEV1 - Development kit(nrf51-ek).
 BOARD_DEV2 - New development kit(nrf51-dk).

 If board type is not defined then it would default to production board.
```sh
echo "Debug version - debug symbols and no optimization"
make debug
echo "Production version - Optimized for size."
make release
echo "Debug version for development kit"
CFLAGS=-DBOARD_AURA make debug
```

## Flash
To flash you must have issued make command to build first and jlink installed in your system.

```sh
echo "Flash the softdevice first"
make flash-softdevice
echo "Programs the app into flash"
make flash

echo "Erase all contents from the flash"
make erase-all

echo "Recover from bad firmware"
make recover
```

### Jlink installation
To be updated

## DFU
To be updated.
