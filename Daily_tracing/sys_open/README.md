

\# trace do\_sys\_open



Use ftrace tool watch function call.



type following instruction in the shell.

```bash

T=/sys/kernel/debug/tracing

echo function\_graph > $T/current\_tracer

echo do\_sys\_open > $T/set\_graph\_function

echo "kallsyms\_\* > $T/set\_ftrace\_notrace

echo 0 > $T/max\_graph\_depth

echo 1 > $T/tracing\_on

ls >/dev/null

echo 0 > $T/tracing\_on

sed -n '1,120p' $T/trace

```

result in do\_sys\_open.png



\## Use printk to show file name and path

edit /fs/open.c

```c

static int do\_sys\_openat2(int dfd, const char \_\_user \*filename,

&nbsp;                         struct open\_how \*how)

{

&nbsp;   if (current->pid == 1) {   

&nbsp;       char fname\[128] = {0};

&nbsp;       long copied = strncpy\_from\_user(fname,  filename, sizeof(fname)-1);

&nbsp;   if (copied > 0)

&nbsp;       printk("\[day4-open] pid=%d flags=%lld path=%s\\n",

&nbsp;               current->pid, how->flags, fname);

&nbsp;   else

&nbsp;       printk("\[day4-open] pid=%d flags=%lld path=<copy-failed>\\n",

&nbsp;               current->pid, how->flags);

}

```

rebuild image

```bash

make -j\&(nproc) bzImage

```

type following in shell

```bash

echo hello >/tmp/a

```

result in show\_path.png



