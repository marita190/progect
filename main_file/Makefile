CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lm

all: main

main:
	$(CC) $(CFLAGS) main.c filter.c -o main $(LDFLAGS)

tests:
	$(CC) $(CFLAGS) tests.c filter.c -o tests $(LDFLAGS)
	./tests

clean:
	rm -f main tests
