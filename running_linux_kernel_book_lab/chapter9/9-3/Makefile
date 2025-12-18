obj-m += mem_alloc_page.o
obj-m += mem_max_contig.o
obj-m += mem_kmalloc_max.o
KDIR ?= /home/william/linux-kernel/linux

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

