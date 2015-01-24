#
# Makefile to build app firmware targets.
# Handle two explict targets: aura and aura_dev1
#

RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
ifeq (aura_dev1,$(firstword $(MAKECMDGOALS)))
export BOARD_TYPE=PLATFORM_AURA_DEV1
aura_dev1:
	@echo "Building for aurav2"
	make -f Makefile $(RUN_ARGS)
else ifeq (aura,$(firstword $(MAKECMDGOALS)))
export BOARD_TYPE=PLATFORM_AURA
aura:
	@echo "Building for aura"
	make -f Makefile $(RUN_ARGS)
else 
export BOARD_TYPE=PLATFORM_AURA
RUN_ARGS := $(wordlist 1,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))

ifneq ($(RUN_ARGS),)
$(firstword $(MAKECMDGOALS)):
	make -f Makefile $(RUN_ARGS)
endif
endif

default:
	make -f Makefile $(RUN_ARGS)

