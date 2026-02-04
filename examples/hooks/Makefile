CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LDFLAGS = -shared

all: hooks.so

hooks.so: hooks.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f *.so

.PHONY: all clean
