obj-m += wq_echo.o

KDIR ?= /home/william/linux-kernel/linux
PWD  := $(shell pwd)

APP := write_wq_echo
APP_SRC := write_wq_echo.c

CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -static

.PHONY: all kmod app clean

all: kmod app

kmod:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

app: $(APP)

$(APP): $(APP_SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(APP)

