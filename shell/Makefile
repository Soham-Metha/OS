CC	   := emcc
CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x

.PHONY: clean

all: clean shell.wasm

shell.wasm: shell.o ../osapi/osapi.o
	@$(CC) $(CFLAGS) $^ -o $@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

shell.o: shell.c shell.h | $(BUILDS)
	@$(CC) $(CFLAGS) $< -c
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

clean:
	@rm -f *.o shell.wasm
