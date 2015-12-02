CC=gcc
CFLAGS=-g -Wall -Wextra -Wconversion -std=gnu99 -I/usr/local/include/
LDFLAGS=$(CFLAGS) -L/usr/local/lib/ -luv

HELLO_SOURCES=hello-world/server.c
HELLO_OBJECTS=$(HELLO_SOURCES:.c=.o)

all: hello-server

hello-server: $(HELLO_OBJECTS)
	$(CC) $(LDFLAGS) $(HELLO_OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
