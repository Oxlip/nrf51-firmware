SDK_SRCS = app_gpiote.c \
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

APPLICATION_SRCS = ble_lbs.c main.c $(SDK_SRCS)
PROJECT_NAME = uBLE

DEVICE = NRF51
BOARD = BOARD_PCA10001

USE_SOFTDEVICE = s110

SDK_PATH = /opt/nrf51sdk/nrf51822/
MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
TEMPLATE_PATH = $(MAKEFILE_DIR)/nrf51-pure-gcc-setup/template/
SOFTDEVICE = s110_nrf51822_7.0.0_softdevice.hex

CFLAGS = -DDEBUG -g3 -O0 -I .

GDB_PORT_NUMBER = 2331

include $(TEMPLATE_PATH)/Makefile
