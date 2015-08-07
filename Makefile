export BLD_TYPE ?= debug

MAKEFLAGS += --no-print-directory

%:
	@make -C app/$*

%-flash:
	@make -C app/$* flash

%-flash-softdevice:
	@make -C app/$* flash-softdevice

%-bootloader:
	@TARGET_DEVICE_TYPE=$* make -C bootloader

%-bootloader-flash:
	@TARGET_DEVICE_TYPE=$* make -C bootloader flash

%-bootloader-flash-softdevice:
	@TARGET_DEVICE_TYPE=$* make -C bootloader flash-softdevice

clean:
	@echo "Erasing build directory"
	@rm -rf _build
