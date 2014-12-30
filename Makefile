# Project infos
SDK_SRCS =	app_gpiote.c \
		app_button.c \
		app_timer.c \
		app_scheduler.c \
		ble_advdata.c \
		ble_conn_params.c \
		pstorage.c \
		device_manager_peripheral.c \
		ble_bas.c \
		ble_ias.c \
		ble_ias_c.c \
		ble_lls.c \
		ble_tps.c \
		ble_srv_common.c \
		crc16.c \
		softdevice_handler.c \
		ble_debug_assert_handler.c \
		ble_error_log.c

UDEVICE          = uplug

ifeq ($(UDEVICE), uplug)
    CFLAGS      += -DDEVICE_CHARS_NUMBER=1
endif

MAKEFILE_DIR  := $(dir $(lastword $(MAKEFILE_LIST)))
TEMPLATE_PATH  = $(MAKEFILE_DIR)/toolchain/gcc/
SDK_PATH       = $(MAKEFILE_DIR)/nrf51-sdk/

include src/Makefile.$(UDEVICE)

PROJECT_NAME     = uBLE
APPLICATION_SRCS = $(DEVICE_SRCS) device.c main.c $(SDK_SRCS)
SOURCE_PATHS    += src
LIBRARY_PATHS   += include

# Device and build infos

DEVICE         = NRF51
BOARD          = BOARD_PCA10001
USE_SOFTDEVICE = s110
SOFTDEVICE     = external/s110_nrf51822_7.0.0_softdevice.hex

# Gdb infos

GDB_PORT_NUMBER = 2331

all:

release: CFLAGS+=-O2
release: all

debug: CFLAGS+=-DDEBUG -g3 -O0 -I .
debug: all

include $(TEMPLATE_PATH)/Makefile
