export BLD_TYPE ?= debug

MAKEFLAGS += --no-print-directory

%:
	@make -C app/$*

%-mail:
	@make -C app/$* mail

%-flash:
	@make -C app/$* flash

%-flash-softdevice:
	@make -C app/$* flash-softdevice

%-flash-erase-all:
	@make -C app/$* erase-all

%-start-debug:
	@make -C app/$* start-debug

%-bootloader:
	@TARGET_DEVICE_TYPE=$* make -C bootloader

%-bootloader_flash:
	@TARGET_DEVICE_TYPE=$* make -C bootloader flash

%-bootloader-flash-softdevice:
	@TARGET_DEVICE_TYPE=$* make -C bootloader flash-softdevice

build-tags:
	@echo "Building cscope and ctag files"
	@find ./ -type f \( -iname \*.c -o -iname \*.h \) > cscope.files
	@cscope -b
	@ctags -L cscope.files

clean:
	@echo "Erasing build directory"
	@rm -rf _build
