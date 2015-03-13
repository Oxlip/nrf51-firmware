MAKEFILE_DIR    := $(dir $(lastword $(MAKEFILE_LIST)))

DEVICE_SRCS     = lyra.c ble.c

PROJECT_NAME    = lyra
SOURCE_PATHS    += $(MAKEFILE_DIR)/app/lyra
LIBRARY_PATHS   += $(MAKEFILE_DIR)/app/lyra

# Device and build infos

DEVICE          = NRF51
BOARD           = BOARD_PCA10001
USE_SOFTDEVICE  = s130
SOFTDEVICE      = $(MAKEFILE_DIR)/lib/s130_nrf51822_0.9.0-1.alpha_softdevice.hex

include ../../Makefile
