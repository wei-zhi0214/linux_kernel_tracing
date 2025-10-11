CC=gcc
CFLAGS=-O2 -Wall

all: limiter memhog cpuhog

limiter: limiter.c
	$(CC) $(CFLAGS) -o limiter limiter.c

memhog: memhog.c
	$(CC) $(CFLAGS) -o memhog memhog.c

cpuhog: cpuhog.c -lm
	$(CC) $(CFLAGS) -o cpuhog cpuhog.c -lm

clean:
	rm -f limiter memhog cpuhog

