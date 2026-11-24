CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
LDLIBS = -lm

.PHONY: all test clean

all: entropy test_bitio

entropy: entropy.c
	$(CC) $(CFLAGS) entropy.c -o entropy $(LDLIBS)

test_bitio: bitio.c bitio.h tests/test_bitio.c
	$(CC) $(CFLAGS) -I. bitio.c tests/test_bitio.c -o test_bitio

test: all
	sh tests/run.sh

clean:
	rm -f entropy test_bitio test.bin
