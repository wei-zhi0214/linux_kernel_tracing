obj-m += timer_kthread_lab.o

KDIR ?= /home/william/linux-kernel/linux
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

