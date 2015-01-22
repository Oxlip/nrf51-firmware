#
# Makefile to build app firmware targets.
# Handle two explict targets: aura and aurav2
#

RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
ifeq (aurav2,$(firstword $(MAKECMDGOALS)))
export BOARD_TYPE=PLATFORM_AURA_V2
aurav2: 
	@echo "Building for aurav2"
	make -f Makefile $(RUN_ARGS)
else ifeq (aura,$(firstword $(MAKECMDGOALS)))
export BOARD_TYPE=PLATFORM_AURA
aura: 
	@echo "Building for aura"
	make -f Makefile $(RUN_ARGS)
else 
RUN_ARGS := $(wordlist 1,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
$(firstword $(MAKECMDGOALS)):
	make -f Makefile $(RUN_ARGS)

endif


