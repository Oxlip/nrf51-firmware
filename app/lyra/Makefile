DEVICE_SRCS     = lyra.c ble.c ble_peer.c battery.c ../../drivers/lis2dh.c

PROJECT_NAME    = lyra
SOURCE_PATHS    += $(REPO_ROOT)/app/lyra
LIBRARY_PATHS   += $(REPO_ROOT)/app/lyra

# Device and build infos

DEVICE          = NRF51
DEVICE_VARIANT  = xxac
TARGET_CHIP     = nrf51822
USE_SOFTDEVICE  = s130
SOFTDEVICE      = $(REPO_ROOT)/softdevice/s130_nrf51_1.0.0_softdevice.hex

include ../../Makefile.common
