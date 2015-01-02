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
     --dfu/         Device Firmware Update.
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
```sh
echo "Debug version - debug symbols and no optimization"
make debug
echo "Production version - Optimized for size (<20K)"
make release         
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
