CC      = cc
CFLAGS  = -Wall -Wextra -Wpedantic -std=gnu11
LDFLAGS = -pthread
TARGETS = prodcons

.PHONY: all clean

all: $(TARGETS)

prodcons: prodcons.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS) numbers.txt
