# Linux kernel practice

This repo is for learning linux kernel.
I'm using qemu simulator and GDB tracing linux source code.
And modify kernel source code, and then check out the understanding for linux kernel.
My reference book is running linux kernel pulish by笨叔.

# Building envirment
package installation
```c
sudo apt install build-essential flex bison libssl-dev libelf-dev libncurses-dev
libncurses5-dev libncursesw5-dev pkg-config
```
download linux kernel source code, chose version you want to trace.
this repo dosen't include linux source code, you should download yourself.
```c
git clone --depth=10 https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
```
compile
```c
make x86_64_defconfig
make -j$(nproc) bzImage
```
using script to enable debug feature.
```c
#!/bin/bash
# 一鍵開啟 Kernel 練習必備設定
# 使用方法： ./enable-kernel-debug.sh && make olddefconfig

# 確保在 kernel 原始碼根目錄執行
cd "$(dirname "$0")"

# Debug 符號
scripts/config --enable CONFIG_DEBUG_INFO
scripts/config --disable CONFIG_DEBUG_INFO_REDUCED
scripts/config --disable CONFIG_DEBUG_INFO_SPLIT
scripts/config --enable CONFIG_DEBUG_INFO_DWARF4   # 或 DWARF5，看環境支援
scripts/config --enable CONFIG_KALLSYMS
scripts/config --enable CONFIG_KALLSYMS_ALL

# ftrace 與 tracing
scripts/config --enable CONFIG_FTRACE
scripts/config --enable CONFIG_FUNCTION_TRACER
scripts/config --enable CONFIG_FUNCTION_GRAPH_TRACER
scripts/config --enable CONFIG_STACK_TRACER
scripts/config --enable CONFIG_DYNAMIC_FTRACE
scripts/config --enable CONFIG_TRACING
scripts/config --enable CONFIG_TRACEPOINTS

# KGDB / GDB 支援
scripts/config --enable CONFIG_GDB_SCRIPTS
scripts/config --enable CONFIG_KGDB
scripts/config --enable CONFIG_KGDB_SERIAL_CONSOLE

# Early printk / console
scripts/config --enable CONFIG_EARLY_PRINTK
scripts/config --enable CONFIG_SERIAL_8250
scripts/config --enable CONFIG_SERIAL_8250_CONSOLE

# Device Tree / Virtio（QEMU 測試用）
scripts/config --enable CONFIG_DEVTMPFS
scripts/config --enable CONFIG_DEVTMPFS_MOUNT
scripts/config --enable CONFIG_VIRTIO_PCI
scripts/config --enable CONFIG_VIRTIO_BLK
```
make image
```c
make -j$(nproc) bzImage
```
I use busybox and init script to build initramfs.
i put initramfs in this repo, you don't have to build yourself.
Start QEMU
if you want to use gdb add -S -s.
```c
qemu-system-x86_64 -m 1024 \
  -kernel ./linux/arch/x86/boot/bzImage \
  -initrd ./rootfs.cpio.gz \
  -append "console=ttyS0 earlycon=uart8250,io,0x3f8,115200 loglevel=7 nokaslr" \
  -nographic -s -S```
```
# Use GDB
go to the linux directory and start vmlinux.
```c
cd ~/linux
gdb vmlinux
```
and then type
```c
target remote :1234
```
you can use GDB now!

