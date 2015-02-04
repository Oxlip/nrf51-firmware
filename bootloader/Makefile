MAKEFILE_DIR    := ../$(dir $(lastword $(MAKEFILE_LIST)))

TEMPLATE_PATH   := $(MAKEFILE_DIR)/toolchain/gcc/
SDK_PATH        = $(MAKEFILE_DIR)/sdk/nrf51822/

LIBRARY_PATHS   += $(MAKEFILE_DIR)/app/include
LIBRARY_PATHS   += $(SDK_PATH)/Include/bootloader_dfu/

BOARD           := BOARD_NRF6310
USE_SOFTDEVICE  := s110
DEVICE          = NRF51
SOFTDEVICE      = $(MAKEFILE_DIR)/lib/s110_nrf51822_7.1.0_softdevice.hex

OUTPUT_NAME     = bootloader

INCLUDEPATHS    +=  -I"$(SDK_PATH)Include/ble" \
                    -I"$(SDK_PATH)Include/app_common" \
                    -I"$(SDK_PATH)Include/s110" \
                    -I"$(SDK_PATH)Include/sd_common" \
                    -I"$(SDK_PATH)Include/ble/ble_services" \
                    -I"./include/ble_transport" \
                    -I"./include"

# SDK files needed for all the devices
SDK_SRCS =  $(SDK_PATH)Source/ble/ble_services/ble_dfu.c \
            $(SDK_PATH)Source/app_common/app_timer.c \
            $(SDK_PATH)Source/app_common/pstorage.c \
            $(SDK_PATH)Source/app_common/hci_mem_pool.c \
            $(SDK_PATH)Source/app_common/app_scheduler.c \
            $(SDK_PATH)Source/app_common/app_gpiote.c \
            $(SDK_PATH)Source/app_common/crc16.c \
            $(SDK_PATH)Source/nrf_delay/nrf_delay.c \
            $(SDK_PATH)Source/ble/ble_conn_params.c \
            $(SDK_PATH)Source/ble/ble_advdata.c \
            $(SDK_PATH)Source/ble/ble_services/ble_srv_common.c \
            $(SDK_PATH)Source/ble/softdevice_handler.c \
            $(SDK_PATH)Source/bootloader_dfu/bootloader.c \
            $(SDK_PATH)Source/bootloader_dfu/dfu_dual_bank.c \
            $(SDK_PATH)Source/bootloader_dfu/dfu_transport_ble.c \
            $(SDK_PATH)Source/bootloader_dfu/bootloader_util_gcc.c 

APPLICATION_SRCS = main.c $(SDK_SRCS)

# Gdb infos
GDB_PORT_NUMBER = 2331

LDFLAGS     += --specs=nano.specs -u _printf_float

# Enable all warnings and treat warnings as error.
CFLAGS      += -Werror -Wall

# Enable link time optimization.
CFLAGS      += -flto

ASMFLAGS    += -D__HEAP_SIZE=16 -D__STACK_SIZE=2048
CFLAGS      += -DPSTORAGE_RAW_MODE_ENABLE -DBOOTLOADER_BANKED 

BOOTLOADER  = 1
include $(TEMPLATE_PATH)/Makefile

release: CFLAGS+=-Os
release: all

debug: CFLAGS+= -DDEBUG -g3 -Os
debug: LDFLAGS+=-g3 -lnosys
debug: all

all:
