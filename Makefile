# Project infos
SDK_SRCS =  app_gpiote.c \
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

COMMON_SRCS     = platform.c ble_common.c fault.c

UDEVICE         = uplug

ifeq ($(UDEVICE), uplug)
    CFLAGS      += -DDEVICE_CHARS_NUMBER=1
endif

MAKEFILE_DIR    := $(dir $(lastword $(MAKEFILE_LIST)))

TEMPLATE_PATH   = $(MAKEFILE_DIR)/toolchain/gcc/
SDK_PATH        = $(MAKEFILE_DIR)/sdk/nrf51822/

LIBRARY_PATHS   += $(MAKEFILE_DIR)/app/include

APPLICATION_SRCS = $(DEVICE_SRCS) $(COMMON_SRCS) $(SDK_SRCS)

# Gdb infos
GDB_PORT_NUMBER = 2331

all:

release: CFLAGS+=-O2
release: all

debug: CFLAGS+=-DDEBUG -g3 -O0 -I .
debug: all

include $(TEMPLATE_PATH)/Makefile
