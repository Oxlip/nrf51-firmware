MAKEFILE_DIR    := $(dir $(lastword $(MAKEFILE_LIST)))

DEVICE_SRCS     = ble.c device.c

PROJECT_NAME    = aura
SOURCE_PATHS    += $(MAKEFILE_DIR)/app/aura
LIBRARY_PATHS   += $(MAKEFILE_DIR)/app/aura

# Device and build infos

DEVICE          = NRF51
BOARD           = BOARD_PCA10001
USE_SOFTDEVICE  = s110
SOFTDEVICE      = $(MAKEFILE_DIR)/lib/s110_nrf51822_7.1.0_softdevice.hex

include ../../Makefile