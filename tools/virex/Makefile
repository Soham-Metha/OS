#* G = GLOBAL LIBS, C = COMPILER, A = ASSEMBLER, M = MACHINE
BUILDS := ./build

A_EXEC := $(BUILDS)/sasm
M_EXEC := $(BUILDS)/virex

_G_DIR := ./src/libs
_A_DIR := ./src/SASM
_M_DIR := ./src/VM

A_MAIN := $(_A_DIR)/sasm_main.c
M_MAIN := $(_M_DIR)/virex_main.c

G_CODE := $(wildcard   $(_G_DIR)/*.c)
A_CODE := $(filter-out $(A_MAIN), 	$(wildcard $(_A_DIR)/*.c) )
M_CODE := $(filter-out $(M_MAIN), 	$(wildcard $(_M_DIR)/*.c) )

CC	   := clang
CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x
LIBS   := $(addprefix -I, $(wildcard ./include/*/)) -I ../../

.PHONY: clean sasm virex all

all 	:	$(A_EXEC) 	$(M_EXEC)
sasm	: 	$(A_EXEC)
virex	:	$(A_EXEC) 	$(M_EXEC)

define BUILD_RULE
$1: $2 $(G_CODE) | $(BUILDS)
	@$(CC) $$^ $(CFLAGS) $(LIBS) -o $$@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $$@ ] \e[0m\n\n"
endef

$(eval $(call BUILD_RULE, $(A_EXEC),  $(A_MAIN) $(A_CODE)))
$(eval $(call BUILD_RULE, $(M_EXEC),  $(M_MAIN) $(M_CODE) $(A_CODE)))

$(BUILDS):
	@mkdir -p $@

clean: | $(BUILDS)
	@rm -f $(BUILDS)/*
	@printf  "\n\e[36m  CLEANED ALL OBJECT FILES AND EXECUTABLES	\e[0m\n\n"
