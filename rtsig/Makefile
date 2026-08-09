CC      = cc
CFLAGS  = -Wall -Wextra -Wpedantic -std=gnu11
TARGETS = receiver sender

.PHONY: all clean

all: $(TARGETS)

receiver: receiver.c
	$(CC) $(CFLAGS) -o $@ $<

sender: sender.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
