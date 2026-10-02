CC      = cc
CFLAGS  = -Wall -Wextra -Wpedantic -std=gnu11
LDFLAGS = -pthread
TARGETS = gameoflife

.PHONY: all clean

all: $(TARGETS)

gameoflife: gameoflife.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)
