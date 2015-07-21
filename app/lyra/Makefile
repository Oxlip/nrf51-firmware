DEVICE_SRCS     = lyra.c ../lis2dh.c ble.c ble_peer.c battery.c

PROJECT_NAME    = lyra
SOURCE_PATHS    += $(REPO_ROOT)/app/lyra
LIBRARY_PATHS   += $(REPO_ROOT)/app/lyra

# Device and build infos

DEVICE          = NRF51
DEVICE_VARIANT  = xxac
TARGET_CHIP     = nrf51822
BOARD           = BOARD_PCA10001
USE_SOFTDEVICE  = s130
SOFTDEVICE      = $(REPO_ROOT)/lib/s130_nrf51_1.0.0_softdevice.hex

include ../../Makefile
