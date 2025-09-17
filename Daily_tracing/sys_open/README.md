

# trace do\_sys\_open



Use ftrace tool watch function call.



type following instruction in the shell.

```bash

T=/sys/kernel/debug/tracing

echo function_graph > $T/current_tracer

echo do_sys_open > $T/set_graph_function

echo "kallsyms_* > $T/set_ftrace_notrace

echo 0 > $T/max_graph_depth

echo 1 > $T/tracing_on

ls >/dev/null

echo 0 > $T/tracing_on

sed -n '1,120p' $T/trace

```

result in do_sys_open.png



## Use printk to show file name and path

edit /fs/open.c

```c

static int do\_sys\_openat2(int dfd, const char __user *filename,

                         struct open_how *how)

{

   if (current->pid == 1) {   

       char fname[128] = {0};

       long copied = strncpy_from_user(fname,  filename, sizeof(fname)-1);

   if (copied > 0)

       printk("[day4-open]pid=%d flags=%lld path=%s\n",

               current->pid, how->flags, fname);

   else

       printk("[day4-open] pid=%d flags=%lld path=<copy-failed>\n",

               current->pid, how->flags);

}

```

rebuild image

```bash

make -j&(nproc) bzImage

```

type following in shell

```bash

echo hello >/tmp/a

```

result in show_path.png




