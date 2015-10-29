# This Makefile is for building and testing under Linux.
# Once everything works there, you probably want to move
# variable_queue.h into kern/inc and/or user/inc.

VQTEST=vqtest
VHTEST=vhtest
AQTEST=aqtest
CC=gcc
CFLAGS = -g -fno-strict-aliasing -Wall -gdwarf-2 -Werror -m32

all: $(VQTEST) $(VHTEST) $(AQTEST)

$(VQTEST): $(VQTEST).c variable_queue.h
	$(CC) $(CFLAGS) $< -o $@

$(AQTEST): $(AQTEST).c array_queue.c array_queue.h
	$(CC) $(CFLAGS) $^ -o $@

$(VHTEST): $(VHTEST).c int_hash.c variable_queue.h variable_htable.h
	$(CC) $(CFLAGS) $^ -o $@
.PHONY: clean

clean:
	rm -f $(VQTEST) $(VHTEST) $(AQTEST)
