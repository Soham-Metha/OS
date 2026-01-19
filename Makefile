BUILDS := ./build

CC	   := emcc -s -sMINIFY_HTML=0
CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x
LIBS   := -I ./include

SRC_DIR   := ./src
MAIN_FILE := $(SRC_DIR)/main.c
EXEC_FILE := $(BUILDS)/a.js
SRC_FILES := $(filter-out $(MAIN_FILE), 	$(wildcard $(SRC_DIR)/*.c) )

all: $(EXEC_FILE)

run_all: all
	@python3 -m http.server 8000

$(BUILDS):
	@mkdir -p $@

clean: | $(BUILDS)
	@rm -f $(BUILDS)/*
	@printf  "\n\e[36m  CLEANED ALL OBJECT FILES AND EXECUTABLES	\e[0m\n\n"

define BUILD_RULE
$1: $2 | $(BUILDS)
	@$(CC) $$^ $(CFLAGS) $(LIBS) -o $$@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $$@ ] \e[0m\n\n"
endef

$(eval $(call BUILD_RULE, $(EXEC_FILE), $(MAIN_FILE) $(SRC_FILES)))