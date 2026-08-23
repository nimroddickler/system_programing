CC      = cc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11
TARGETS = stopwatch

.PHONY: all clean

all: $(TARGETS)

stopwatch: stopwatch.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
