DEVICE_SRCS     = $(wildcard *.c) ../../drivers/cs_78m6610_lmu.c

PROJECT_NAME    = aura
SOURCE_PATHS    += $(REPO_ROOT)/app/aura
LIBRARY_PATHS   += $(REPO_ROOT)/app/aura

# Device and build infos

DEVICE          = NRF51
DEVICE_VARIANT  = xxac
TARGET_CHIP     = nrf51822
USE_SOFTDEVICE  = s110
SOFTDEVICE      = $(REPO_ROOT)/softdevice/s110_nrf51_8.0.0_softdevice.hex

NEED_SPI_SLAVE	= TRUE

include ../../Makefile.common
