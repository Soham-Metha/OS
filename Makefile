BUILDS := ./build

CC	   := emcc
CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x
LIBS   := -I ./osapi
LDFLAG := -sMINIFY_HTML=0

EXEC_FILE := $(BUILDS)/shell.js

.PHONY: clean

all: clean $(EXEC_FILE)

run_all: all
	@python3 -m http.server 8000

$(BUILDS):
	@mkdir -p $@

clean: | $(BUILDS)
	@rm -f $(BUILDS)/*
	@printf  "\n\e[36m  CLEANED ALL OBJECT FILES AND EXECUTABLES	\e[0m\n\n"


_OSAPI := $(BUILDS)/osapi.o
_SHELL := $(BUILDS)/shell.o

osapi.o:  osapi/osapi.c osapi/osapi.h osapi/os.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) $< -c

shell.o: shell/shell.c shell/shell.h | $(BUILDS)
	@$(CC) $(CFLAGS) $(LIBS) $< -c

$(_OSAPI): osapi.o
	@mv ./osapi.o $(BUILDS)
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(_SHELL): shell.o
	@mv ./shell.o $(BUILDS)
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

$(EXEC_FILE): $(_OSAPI) $(_SHELL)
	@$(CC) $(CFLAGS) $(LIBS) $(LDFLAG) $^ -o $@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"
