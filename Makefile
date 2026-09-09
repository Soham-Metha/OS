export PREFIX := ~/Downloads/LLVM-23.1.0-Linux-X64/
export PATH := $(PREFIX)/bin:$(PATH)

.ONESHELL:
SHELL  := /bin/bash
BUILDS := ./build
SRC    := ./src

CC     := clang-23

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
NAT_TARGET := --target=i686-elf -m32
CFLAGS     += $(NAT_TARGET)
CFLAGS     += -mno-sse -mno-sse2 -mno-mmx -msoft-float
LD         := ld.lld
LFLAGS     := -m elf_i386 -T $(SRC)/platform/i386/native.ld

all: clean $(_ISO)

run_all: all
	@qemu-system-i386 -enable-kvm -drive format=raw,file="$(_ISO)" -vga std

$(_HAL): $(SRC)/hal/i386/boot.c $(SRC)/hal/hal.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

# ============================================================
else # Browser / WASM target
# ============================================================

WEB_TARGET := --target=wasm32-unknown-unknown
CFLAGS     += $(WEB_TARGET)
LD         := wasm-ld
LFLAGS     := --allow-undefined --no-entry --initial-memory=33554432 --global-base=524288 -z stack-size=524288
LFLAGS     += --export=kernelMain --export=kernel_irq_wrapper --export-table

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
	@$(LD) $(LFLAGS) $^ -o $@ && \
	printf "\e[32m		[ LINK  COMPLETED ]\t: [ $@ ] \e[0m\n\n"

# ============================================================
# Native executable
# ============================================================

$(_ISO): $(_NATIVE_KERNEL)
	@grub-mkrescue -o $@ $(BUILDS)/OS/ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_NATIVE_KERNEL): $(_NATIVE_BOOT_A) $(_OSAPI) $(_SHELL) $(_KERN) $(_HAL) $(_ITR) $(_EVENT)
	@$(LD) $(LFLAGS)  $^ -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_NATIVE_BOOT_A): $(SRC)/hal/i386/boot.S | $(BUILDS)
	@$(CC) $(NAT_TARGET) -c $< -o $@ && \
	printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"
