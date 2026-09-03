export PREFIX := ./tools/cross
export PATH := $(PREFIX)/bin:$(PATH)

.ONESHELL:
SHELL  := /bin/bash
BUILDS := ./build
SRC    := ./src

NAT_CC := i686-elf-gcc
NAT_AS := i686-elf-as
NAT_LD := i686-elf-ld

CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x
CFLAGS += -ffreestanding -fno-builtin -g
LIBS   := -I $(SRC)

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
	@rm -f $(BUILDS)/*.o $(_NATIVE_KERNEL) && \
	printf  "\n\e[36m  CLEANED ALL OBJECT FILES AND EXECUTABLES	\e[0m\n\n"

# ============================================================
ifeq ($(TARGET),native) # Native target
# ============================================================

ifeq ($(COMPILER),clang)
CC       := clang-15
NAT_CC   := clang-15
NAT_AS   := clang-15
NAT_LD   := ld.lld
NAT_CFLAGS := --target=i686-elf -m32
CFLAGS   += $(NAT_CFLAGS)
else
CC     := $(NAT_CC)
endif

all: clean $(_ISO)

run_all: all
	@qemu-system-i386 -enable-kvm -drive format=raw,file="$(_ISO)" -vga std

$(_HAL): $(SRC)/hal/i386/boot.c $(SRC)/hal/hal.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

# ============================================================
else # Browser / WASM target
# ============================================================

ifeq ($(COMPILER),clang)
CC     := clang-15
LD     := wasm-ld
CFLAGS += --target=wasm32-unknown-unknown
LFLAGS := --allow-undefined --no-entry --initial-memory=33554432 --global-base=1024 -z stack-size=16384
LFLAGS += --export=kernelMain --export=kernel_irq_wrapper --export-table
else
CC     := emcc
LD     := emcc
CFLAGS += -matomics -mbulk-memory
LFLAGS := -sMINIFY_HTML=0 -Wl,--no-entry -s INITIAL_MEMORY=32MB -s STANDALONE_WASM=1 -g
LFLAGS += -s EXPORTED_FUNCTIONS=['_kernelMain','_kernel_irq_wrapper'] -s ERROR_ON_UNDEFINED_SYMBOLS=0
endif

all: clean $(EXEC_FILE)

run_all: all
	@python3 -m http.server 8000

$(_HAL): $(SRC)/hal/wasm32/hal_browser.c $(SRC)/hal/hal.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

endif

# ============================================================
# Common objects
# ============================================================

$(_EVENT): $(SRC)/osapi/gfx/event.c $(SRC)/common/event.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_ITR): $(SRC)/kernel/interrupt.c $(SRC)/kernel/interrupt.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_KERN): $(SRC)/kernel/kernel.c $(SRC)/kernel/kernel.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_OSAPI): $(SRC)/osapi/osapi.c $(SRC)/osapi/osapi.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_SHELL): $(SRC)/apps/shell.c $(SRC)/apps/shell.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

# ============================================================
# WASM executable
# ============================================================

$(EXEC_FILE): $(_OSAPI) $(_SHELL) $(_KERN) $(_HAL) $(_ITR) $(_EVENT)
	@source ./tools/emsdk/emsdk_env.sh
	@$(LD) $(LFLAGS) $^ -o $@ && \
	printf "\e[32m		[ LINK  COMPLETED ]\t: [ $@ ] \e[0m\n\n"

# ============================================================
# Native executable
# ============================================================

$(_ISO): $(_NATIVE_KERNEL)
	@grub-mkrescue -o $@ $(BUILDS)/OS/ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_NATIVE_KERNEL): $(_NATIVE_BOOT_A) $(_OSAPI) $(_SHELL) $(_KERN) $(_HAL) $(_ITR) $(_EVENT)
	@$(NAT_LD) -m elf_i386 -T $(SRC)/platform/i386/native.ld  $^ -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_NATIVE_BOOT_A): $(SRC)/hal/i386/boot.S | $(BUILDS)
	@$(NAT_AS) $(NAT_CFLAGS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"
