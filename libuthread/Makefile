# Target library
lib := libuthread.a
CC := gcc
CFLAGS := -Wall -Wextra -Werror

all: $(lib)

# TODO
libuthread.a: queue.o uthread.o context.o
	ar rcs libuthread.a queue.o uthread.o context.o

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c -o queue.o queue.c

uthread.o: uthread.c uthread.h private.h queue.h
	$(CC) $(CFLAGS) -c -o uthread.o uthread.c

context.o: context.c private.h
	$(CC) $(CFLAGS) -c -o context.o context.c

clean:
	rm -f libuthread.a queue.o uthread.o context.o
