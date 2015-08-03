define colorecho
	printf "%-15s" $2
	@tput setaf $1

	printf "%-30s" $(notdir $3)
	@tput sgr0
	printf "%-50s\n" $(realpath $(dir $3))
endef

REPO_ROOT       := $(dir $(lastword $(MAKEFILE_LIST)))

SDK_PATH        = $(REPO_ROOT)/sdk/9.0/
SDK_COMPONENT_DIR = $(SDK_PATH)/components/

MK := mkdir
RM := rm -rf

#echo suspend
ifeq ("$(VERBOSE)","1")
NO_ECHO := 
else
NO_ECHO := @
MAKEFLAGS += -s
endif

BUILD_TIME = $(shell date +"%d%b%y_%H%M")

GNU_INSTALL_ROOT := /usr/
GNU_VERSION := 4.8.3
GNU_PREFIX := arm-none-eabi

# Toolchain commands
CC := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc"
AS := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as"
AR := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar" -r
LD := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld"
NM := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm"
OBJDUMP := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump"
OBJCOPY := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy"
SIZE := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size"
GDB := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gdb"

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

#convert softdevice name to uppercase
SOFTDEVICE_UPPER = `echo $(USE_SOFTDEVICE) | tr a-z A-Z`

#source common to all targets
C_SOURCE_FILES += \
$(abspath $(REPO_ROOT)/app/platform.c) \
$(abspath $(REPO_ROOT)/app/fault.c) \
$(abspath $(REPO_ROOT)/app/ble_common.c) \
$(abspath $(REPO_ROOT)/app/ble_ss.c) \
$(abspath $(REPO_ROOT)/app/smbus.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/button/app_button.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/timer/app_timer.c) \
$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/hal/nrf_delay.c) \
$(abspath $(SDK_COMPONENT_DIR)/ble/common/ble_advdata.c) \
$(abspath $(SDK_COMPONENT_DIR)/ble/common/ble_conn_params.c) \
$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/common/nrf_drv_common.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/gpiote/app_gpiote.c) \
$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/pstorage/pstorage.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/scheduler/app_scheduler.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/util/nrf_assert.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/uart/retarget.c) \
$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/uart/app_uart_fifo.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/fifo/app_fifo.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/crc16/crc16.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/trace/app_trace.c) \
$(abspath $(SDK_COMPONENT_DIR)/ble/common/ble_srv_common.c) \
$(abspath $(SDK_COMPONENT_DIR)/ble/ble_services/ble_dis/ble_dis.c) \
$(abspath $(SDK_COMPONENT_DIR)/ble/ble_services/ble_dfu/ble_dfu.c) \
$(abspath $(SDK_COMPONENT_DIR)/ble/ble_debug_assert_handler/ble_debug_assert_handler.c) \
$(abspath $(SDK_COMPONENT_DIR)/toolchain/system_nrf51.c) \
$(abspath $(SDK_COMPONENT_DIR)/softdevice/common/softdevice_handler/softdevice_handler.c) \
$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/twi_master/incubated/twi_sw_master.c) \
$(abspath $(SDK_COMPONENT_DIR)/ble/ble_error_log/ble_error_log.c) \
$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/ble_flash/ble_flash.c) \
$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/gpiote/nrf_drv_gpiote.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/timer/app_timer_appsh.c) \
$(abspath $(SDK_COMPONENT_DIR)/softdevice/common/softdevice_handler/softdevice_handler_appsh.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu/dfu_app_handler.c) \
$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu/bootloader_util.c) \
$(abspath $(SDK_COMPONENT_DIR)/ble/device_manager/device_manager_peripheral.c)

#assembly files common to all targets
ASM_SOURCE_FILES  = $(abspath $(REPO_ROOT)/app/startup_nrf51.s)

#includes common to all targets
INC_PATHS += -I$(abspath $(CURDIR))
INC_PATHS += -I$(abspath $(REPO_ROOT)/app/)
INC_PATHS += -I$(abspath $(REPO_ROOT)/app/include)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/config)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/scheduler)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/pstorage)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/uart)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/twi_master/incubated)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/ble/common)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/ble/device_manager)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/ble/ble_debug_assert_handler)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/ble/ble_services/ble_dis)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/ble/ble_services/ble_dfu)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/device)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/button)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/timer)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/crc16)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)//libraries/fifo)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/softdevice/$(USE_SOFTDEVICE)/headers)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/gpiote)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/util)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/trace)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/hal)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/toolchain/gcc)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/toolchain/)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/common)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/ble/ble_db_discovery)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/softdevice/common/softdevice_handler)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/ble/ble_error_log)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/drivers_nrf/ble_flash)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/libraries/bootloader_dfu)

# Common platform files
C_SOURCE_FILES += $(DEVICE_SRCS)

ifeq ($(USE_SOFTDEVICE),s130)
CFLAGS += -DUSE_CENTRAL_MODE=1
C_SOURCE_FILES += $(abspath $(SDK_COMPONENT_DIR)/ble/ble_services/ble_bas_c/ble_bas_c.c)
INC_PATHS += -I$(abspath $(SDK_COMPONENT_DIR)/ble/ble_services/ble_bas_c)
endif


BLD_TYPE ?= release
ifeq ($(BLD_TYPE),debug)
CFLAGS+=-DDEBUG -g3
endif

BLD_DIRECTORY = _build

OBJECT_DIRECTORY = $(BLD_DIRECTORY)/$(BLD_TYPE)/
LISTING_DIRECTORY = $(BLD_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(BLD_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF51
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Werror
CFLAGS += -flto
CFLAGS += -O3
CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums
CFLAGS += -DBUILD_TIME='"$(BUILD_TIME)"'

CFLAGS += -DSWI_DISABLE0
CFLAGS += -D$(BOARD)
CFLAGS += -D$(SOFTDEVICE_UPPER)


# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(LINKER_SCRIPT_DIR) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys -u _printf_float

# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF51
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD

ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -D$(BOARD)
ASMFLAGS += -D$(SOFTDEVICE_UPPER)


C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )
C_DEPS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.d) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

OUTPUT_FILENAME := $(PROJECT_NAME)_$(TARGET_CHIP)_$(DEVICE_VARIANT)_$(USE_SOFTDEVICE)
LINKER_SCRIPT_DIR := $(abspath $(REPO_ROOT)/ldscripts)
LINKER_SCRIPT := nrf51_$(USE_SOFTDEVICE)_$(DEVICE_VARIANT).ld

## Create build directories
$(BUILD_DIRECTORIES):
	$(MK) -p $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	$(call colorecho, 5, "Compiling ", $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	$(call colorecho, 6, "Assembling ", $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<

ELF_OUTPUT = $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).elf
HEX_OUTPUT = $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
BIN_OUTPUT = $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin
UICR_BIN_OUTPUT = $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME)-uirc.bin

# Link
$(ELF_OUTPUT): $(BUILD_DIRECTORIES) $(OBJECTS)
	$(call colorecho, 3, "Linking ", $@)
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

## Create binary .bin file from the .elf file
$(BIN_OUTPUT): $(ELF_OUTPUT)
	$(call colorecho, 2, "Generating ", $@)
	$(NO_ECHO)$(OBJCOPY) -O binary $(ELF_OUTPUT) $@
ifdef BOOTLOADER
	$(call colorecho, 8, "Generating bootloader ", $@)
	$(NO_ECHO)$(OBJCOPY) -O binary $(OBJ_OPT_UIRC) $(ELF_OUTPUT) $(UICR_BIN_OUTPUT)
endif

## Create binary .hex file from the .elf file
$(HEX_OUTPUT): $(ELF_OUTPUT)
	$(call colorecho, 2, "Generating ", $@)
	$(NO_ECHO)$(OBJCOPY) -O ihex $(ELF_OUTPUT) $@

all: $(ELF_OUTPUT) $(HEX_OUTPUT) $(BIN_OUTPUT)
	$(NO_ECHO)$(SIZE) $(ELF_OUTPUT)

clean: clean_flash_intermediate
	$(RM) $(BUILD_DIRECTORIES)

.DEFAULT_GOAL = all

-include $(C_DEPS)

include $(REPO_ROOT)/Makefile.flash
