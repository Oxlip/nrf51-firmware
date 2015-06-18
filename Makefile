MAKEFILE_DIR    := $(dir $(lastword $(MAKEFILE_LIST)))

TEMPLATE_PATH   = $(MAKEFILE_DIR)/toolchain/gcc/
SDK_PATH        = $(MAKEFILE_DIR)/sdk/nrf51822/

LIBRARY_PATHS   += $(MAKEFILE_DIR)/app/include
LIBRARY_PATHS   += $(SDK_PATH)/Include/bootloader_dfu/

# SDK files needed for all the devices
SDK_SRCS =  app_gpiote.c \
            app_button.c \
            app_timer.c \
            nrf_delay.c \
            app_scheduler.c \
            ble_advdata.c \
            ble_conn_params.c \
            pstorage.c \
            ble_dis.c \
            bootloader_util_gcc.c \
            ble_srv_common.c \
            softdevice_handler.c \
            ble_debug_assert_handler.c \
            ble_error_log.c \
            twi_hw_master.c \
            simple_uart.c

# Common platform files
COMMON_SRCS     = platform.c fault.c ble_common.c smbus.c

ifeq ($(USE_SOFTDEVICE),s130)
CFLAGS += -DUSE_CENTRAL_MODE=1
SDK_SRCS += device_manager_central_s130.c ble_db_discovery.c
COMMON_SRCS += blec_common.c ble_ss.c
endif

ifneq ($(USE_SOFTDEVICE),s130)
SDK_SRCS +=  ble_dfu.c dfu_app_handler.c
COMMON_SRCS += ble_ss.c
endif


APPLICATION_SRCS = $(DEVICE_SRCS) $(COMMON_SRCS) $(SDK_SRCS)

BUILD_TIME		= $(shell date +"%d%b%y_%H%M")

# Gdb infos
GDB_PORT_NUMBER = 2331

LDFLAGS += --specs=nano.specs -u _printf_float

CFLAGS 	+= -DBUILD_TIME='"$(BUILD_TIME)"'

# Enable all warnings and treat warnings as error.
CFLAGS	+= -Werror -Wall

# Enable link time optimization.
CFLAGS	+= -flto


include $(TEMPLATE_PATH)/Makefile

release: CFLAGS+=-Os
release: all

debug: CFLAGS+=-DDEBUG -g3 -Os
debug: LDFLAGS+=-g3 -lnosys
debug: all

all:
	echo $(BUILD_TIME) > $(OUTPUT_PATH)/build_time.txt
