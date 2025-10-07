 

 # Process management

 

 ## Chapter 8 Labs

 

 ### Kernel thread

 

 #### Purpose

 Familar with how linux create kernel thread

 

 ##### Lab requirement

 (1) Write a Kernel module to create kernel thread run on each CPU core.\

 (2) In ever CPU core, print out current CPU state, for example, ARM64 general purpose register etc.\

 (3) In each kernel thread, print out priority of cureent process.

 

 BindCpu.c show how to Bind Cpu to particular CPU.

 And kthread_lab.c is the lab answer. It create kthread to different CPU and print out priority and policy and nice value, cpu register.

 

 Makefile will make kernel module and move the kernel module to initramfs folder.

 create new cpio file.

 restart qemu.

 insmod kernel module.

 use dmesg to check log.

 notice:

 qemu commnad need to add -smp 4 to add 4 CPU core.

 

 ```bash

 qemu-system-x86\_64 -m 1024 \

 ; -kernel ./linux/arch/x86/boot/bzImage \

 ; -initrd ./rootfs.cpio.gz \

 ; -append "console=ttyS0 earlycon=uart8250,io,0x3f8,115200 loglevel=7 nokaslr rdinit=/init" \

 ; -nographic -smp 4

 ```

 result in kthread.png

 

 #### how to create kthread

 First, use kthread_create function to create thread. And then use wake_up_process function function put it in run queue. If you want to terminate kthread, call kthread_stop function.

