# \# Debug tool

# 

# Introduce how to use printk and GDB.

# printk is use like uart to transmit data to terminal.

# like we use printf to debug. but it's not suitable for every scenario. Because it use uart, need to wait hardware finish.

# 

# \## how to use it 

# First of all, you have to edit kernel source code, and compile it.

# restart kernel.

# 

# move to your kernel directory.

# ```bash

# cd linux/init

# ```

# edit kernel source code

# ```bash

# vim main.c

# ```

# I add printk in start\_kernel.

# ```c

# void start\_kernel(void)

# {

# &nbsp;       pr\_info("Hello from start\_kernel (Day3 test)\\n");

# &nbsp;       char \*command\_line;

# &nbsp;       char \*after\_dashes;

# ```

# rebuild kernel

# ```bash

# make -j$(nproc) bzImage

# ```

# restart qemu and result

# !\[image](/Daily\_tracing/printkAndGdb/printk.png)

