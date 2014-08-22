# List all source files the application uses.
SDK_SRCS = nrf_delay.c
APPLICATION_SRCS = main.c $(SDK_SRCS)
PROJECT_NAME = uSense

DEVICE = NRF51
BOARD = BOARD_PCA10001
#SEGGER_SERIAL =

USE_SOFTDEVICE = s110

SDK_PATH = /opt/nrf51sdk/nrf51822/
MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
TEMPLATE_PATH = $(MAKEFILE_DIR)/../nrf51-pure-gcc-setup/template/
SOFTDEVICE = s110_nrf51822_7.0.0_softdevice.hex

CFLAGS = -Os

GDB_PORT_NUMBER = 2331

include $(TEMPLATE_PATH)Makefile
