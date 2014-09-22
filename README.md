bluetooth-firmware
==================

Firmware for Bluetooth

### Target

Different target will be possible. For the moment only uplug is available.

```sh
make rules UDEVICE=uplug
make rules
```

### Debug version - debug symboles and no optimisation

```sh
make debug
```

### Low size version (<20K)

```sh
make release
```

```
repos/
--nrf51-pure-gcc-setup/ gcc version of makefile SDK nordic
--external/ no modified external file
--sdk_modified/ modified file from the nordic SDK
--include/ include (SDK + Private)
--src/ source of the app
```
