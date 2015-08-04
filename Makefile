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
	rm -rf _build
