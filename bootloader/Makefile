DEVICE_SRCS     = main.c dfu_ble_svc.c

PROJECT_NAME    = bootloader-$(TARGET_DEVICE_TYPE)
SOURCE_PATHS    += $(REPO_ROOT)/bootloader
LIBRARY_PATHS   += $(REPO_ROOT)/bootloader

C_SOURCE_FILES += \
$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu/bootloader.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu/bootloader_settings.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu/dfu_init_template.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu/dfu_single_bank.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu/dfu_transport_ble.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/hci/hci_mem_pool.c)

INC_PATHS += -I$(abspath $(REPO_ROOT)/bootloader)
INC_PATHS += -I$(abspath $(REPO_ROOT)/app/$(TARGET_DEVICE_TYPE))
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/hci/)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu/ble_transport/)

# Device and build infos
DEVICE          = NRF51
DEVICE_VARIANT  = xxac
TARGET_CHIP     = nrf51822
ifeq ($(TARGET_DEVICE_TYPE),lyra)
USE_SOFTDEVICE  = s130
SOFTDEVICE      = $(REPO_ROOT)/softdevice/s130_nrf51_1.0.0_softdevice.hex
else
USE_SOFTDEVICE  = s110
SOFTDEVICE      = $(REPO_ROOT)/softdevice/s110_nrf51_8.0.0_softdevice.hex
endif

BOOTLOADER 		:= 1
LINKER_SCRIPT   := nrf51_dfu.ld

ASMFLAGS    += -D__HEAP_SIZE=0x100 -D__STACK_SIZE=2048 -DBOOTLOADER
CFLAGS      += -DBOOTLOADER

include ../Makefile.common
