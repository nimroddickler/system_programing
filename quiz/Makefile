CC      = cc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11
TARGETS = quiz

.PHONY: all clean

all: $(TARGETS)

quiz: quiz.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
