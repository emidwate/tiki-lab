CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I. -I../lab1
LDLIBS = -lm

SRCS = main.c huff.c ../lab1/bitio.c
HEADERS = huff.h ../lab1/bitio.h

.PHONY: all abracadabra test clean

all: huff

huff: $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(SRCS) -o huff $(LDLIBS)

abracadabra: huff
	./huff t tests/abracadabra.txt

test: huff
	sh tests/run.sh

clean:
	rm -f huff *.huf *.dec tests/*.huf tests/*.dec
