obj-m := memstat.o

KDIR ?= $(HOME)/linux-kernel/linux
PWD  := $(shell pwd)

.PHONY: all clean

all:
	$(MAKE) -C $(KDIR) M="$(PWD)" modules

clean:
	$(MAKE) -C $(KDIR) M="$(PWD)" clean

