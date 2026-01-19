CC	   := emcc
CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x

.PHONY: clean

all: clean osapi.o

osapi.o: osapi.c osapi.h | $(BUILDS)
	@$(CC) $(CFLAGS) $< -c
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $@ ] \e[0m\n\n"

clean:
	@rm osapi.o
