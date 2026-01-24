.ONESHELL:
BUILDS := ./build

CC	   := emcc
CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x
LIBS   := -I .
LDFLAG := -sMINIFY_HTML=0 -Wl,--no-entry -s INITIAL_MEMORY=9MB \
  -s STANDALONE_WASM=1 -s EXPORTED_FUNCTIONS=['_main','_kernel_irq',' _kernel_tick'] --js-library ./extras/js_exports.js

SHELL     := /bin/bash
EXEC_FILE := $(BUILDS)/shell.wasm

.PHONY: clean

all: clean $(EXEC_FILE)

run_all: all
	@python3 -m http.server 8000

$(BUILDS):
	@mkdir -p $@

clean: | $(BUILDS)
	@rm -f $(BUILDS)/*
	@printf  "\n\e[36m  CLEANED ALL OBJECT FILES AND EXECUTABLES	\e[0m\n\n"


_HAL   := $(BUILDS)/hal_browser.o
_ITR   := $(BUILDS)/interrupt.o
_KERN  := $(BUILDS)/kernel.o
_OSAPI := $(BUILDS)/osapi.o
_SHELL := $(BUILDS)/shell.o

$(_HAL): hal/hal_browser.c hal/hal.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_ITR): kernel/interrupt.c kernel/interrupt.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_KERN): kernel/kernel.c kernel/kernel.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_OSAPI): osapi/osapi.c osapi/osapi.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_SHELL):  userspace/shell.c userspace/shell.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(EXEC_FILE): $(_OSAPI) $(_SHELL) $(_KERN) $(_HAL) $(_ITR)
	@source ./tools/emsdk/emsdk_env.sh
	@$(CC) $(CFLAGS) $(LIBS) $(LDFLAG) $^ -o $@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"
