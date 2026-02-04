CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LDFLAGS = -shared

all: hello.so

hello.so: hello.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f *.so

.PHONY: all clean
