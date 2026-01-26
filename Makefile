export PREFIX := ./tools/cross
export PATH := $(PREFIX)/bin:$(PATH)

.ONESHELL:
SHELL  := /bin/bash
BUILDS := ./build

NAT_CC := i686-elf-gcc
NAT_AS := i686-elf-as
NAT_LD := i686-elf-ld

CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x
CFLAGS += -ffreestanding  -fno-builtin -I .
LIBS   := 

_HAL   := $(BUILDS)/hal_browser.o
_ITR   := $(BUILDS)/interrupt.o
_KERN  := $(BUILDS)/kernel.o
_OSAPI := $(BUILDS)/osapi.o
_SHELL := $(BUILDS)/shell.o
_EVENT := $(BUILDS)/event.o

_NATIVE_BOOT_A := $(BUILDS)/nat_boot_a.o
_NATIVE_KERNEL := $(BUILDS)/OS/boot/kernel
_ISO           := $(BUILDS)/os.iso
EXEC_FILE      := $(BUILDS)/shell.wasm

.PHONY: clean all run_all

$(BUILDS):
	@mkdir -p $@/OS/boot/grub

clean: | $(BUILDS)
	@rm -f $(BUILDS)/*.o && \
	printf  "\n\e[36m  CLEANED ALL OBJECT FILES AND EXECUTABLES	\e[0m\n\n"

ifeq ($(TARGET),native)
CC     := $(NAT_CC)

all: clean $(_ISO)

run_all: all
	@qemu-system-i386 	-drive format=raw,file="$(_ISO)"  -vga std

$(_HAL): arch/native/boot.c arch/hal.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

else

ifeq ($(COMPILER),clang)
LD     := wasm-ld
CFLAGS += --target=wasm32-unknown-unknown
LFLAGS := --allow-undefined --no-entry --initial-memory=9437184 --global-base=1024 -z stack-size=16384
LFLAGS += --export=main  --export=kernel_irq  --export=kernel_tick --export-table

else
CC     := emcc
LD     := emcc
LFLAGS := -sMINIFY_HTML=0 -Wl,--no-entry -s INITIAL_MEMORY=9MB -s STANDALONE_WASM=1
LFLAGS += -s EXPORTED_FUNCTIONS=['_main','_kernel_irq',' _kernel_tick'] -s ERROR_ON_UNDEFINED_SYMBOLS=0

endif

all: clean $(EXEC_FILE)

run_all: all
	@python3 -m http.server 8000

$(_HAL): arch/browser/hal_browser.c arch/hal.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

endif

$(_EVENT): kernel/event.c kernel/event.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_ITR): kernel/interrupt.c kernel/interrupt.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_KERN): kernel/kernel.c kernel/kernel.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_OSAPI): osapi/osapi.c osapi/osapi.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_SHELL): userspace/shell.c userspace/shell.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(EXEC_FILE): $(_OSAPI) $(_SHELL) $(_KERN) $(_HAL) $(_ITR) $(_EVENT)
	@source ./tools/emsdk/emsdk_env.sh
	@$(LD) $(LFLAGS) $^ -o $@ && \
	printf "\e[32m		[ LINK  COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_ISO): $(_NATIVE_KERNEL)
	@grub-mkrescue -o $@ $(BUILDS)/OS/ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_NATIVE_KERNEL): $(_NATIVE_BOOT_A) $(_OSAPI) $(_SHELL) $(_KERN) $(_HAL) $(_ITR) $(_EVENT)
	@$(NAT_LD) -m elf_i386 -T ./extras/native.ld  $^ -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_NATIVE_BOOT_A):arch/native/boot.S  | $(BUILDS)
	@$(NAT_AS) --32 $< -o $@  && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"
