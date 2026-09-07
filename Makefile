CC = gcc
CFLAGS = -Wall -Wextra -std=c11

scheduler: scheduler.c
	$(CC) $(CFLAGS) scheduler.c -o scheduler

clean:
	rm -f scheduler

.PHONY: clean