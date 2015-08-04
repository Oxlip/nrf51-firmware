DEVICE_SRCS     = main.c

PROJECT_NAME    = bootloader-$(TARGET_DEVICE_TYPE)
SOURCE_PATHS    += $(REPO_ROOT)/bootloader
LIBRARY_PATHS   += $(REPO_ROOT)/bootloader
INC_PATHS       += -I$(abspath $(REPO_ROOT)/app/$(TARGET_DEVICE_TYPE))

# Device and build infos

DEVICE          = NRF51
DEVICE_VARIANT  = xxac
TARGET_CHIP     = nrf51822
ifeq ($(TARGET_DEVICE_TYPE),lyra)
USE_SOFTDEVICE  = s130
SOFTDEVICE      = $(REPO_ROOT)/lib/s130_nrf51_1.0.0_softdevice.hex
else
USE_SOFTDEVICE  = s110
SOFTDEVICE      = $(REPO_ROOT)/lib/s110_nrf51_8.0.0_softdevice.hex
endif

BOOTLOADER 		:= 1
LINKER_SCRIPT   := nrf51_dfu.ld

ASMFLAGS    += -D__HEAP_SIZE=0x100 -D__STACK_SIZE=2048
CFLAGS      += -DBOOTLOADER_BANKED

include ../Makefile.common
