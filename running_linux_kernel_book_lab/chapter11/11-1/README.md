#  Memory Management – Chapter 11 Labs
## **Lab: tasklet*

###  **Purpose**
1. Understand the tasklet mechanism in Linux.
2. Learn how tasklets are scheduled and executed as part of the softirq subsystem.
3. Clarify the execution context and limitations of tasklets.
---

###  **Lab Requirement**
1. Write a kernel module that:
   Initializes a tasklet.
   Schedules the tasklet from a write() file operation.
   Prints the user-provided string inside the tasklet callback.
2. Write a user-space application to test the module.
---
### Background

What is a Tasklet?

A tasklet is a type of softirq bottom half.

It runs in interrupt context:
❌ Cannot sleep
❌ Cannot perform blocking operations

A tasklet is serialized per CPU:
The same tasklet instance will not run concurrently.
Execution is bound to a specific CPU and will not migrate.
Tasklets are implemented on top of the TASKLET_SOFTIRQ.

Linux Softirq Types
```c
enum
{
	HI_SOFTIRQ=0,
	TIMER_SOFTIRQ,
	NET_TX_SOFTIRQ,
	NET_RX_SOFTIRQ,
	BLOCK_SOFTIRQ,
	IRQ_POLL_SOFTIRQ,
	TASKLET_SOFTIRQ,
	SCHED_SOFTIRQ,
	HRTIMER_SOFTIRQ,
	RCU_SOFTIRQ,    /* Preferable RCU should always be the last softirq */

	NR_SOFTIRQS
};
```

### Softirq Execution Flow
From Hardware Interrupt to Softirq
During interrupt handling, Linux defers most work to softirqs.
The main flow:
```
handle_domain_irq()
 ├─ irq_enter()
 ├─ handle_irq_desc()
 └─ irq_exit()
```
Relevant code path:
```
int handle_domain_irq(struct irq_domain *domain,
                      unsigned int hwirq, struct pt_regs *regs)
{
        irq_enter();
        desc = irq_resolve_mapping(domain, hwirq);
        if (likely(desc))
                handle_irq_desc(desc);
        irq_exit();
}
```
irq_exit() and Softirq Invocation
At irq_exit(), the kernel checks whether pending softirqs should be executed immediately:
```
static inline void __irq_exit_rcu(void)
{
        if (!in_interrupt() && local_softirq_pending())
                invoke_softirq();
}

```
Softirqs may be:
Executed immediately via __do_softirq()
Deferred to ksoftirqd if execution would take too long

Softirq Processing Loop
```c
asmlinkage __visible void __softirq_entry __do_softirq(void)
{
restart:
        set_softirq_pending(0);
        local_irq_enable();

        while ((softirq_bit = ffs(pending))) {
                h->action(h);
                pending >>= softirq_bit;
        }

        local_irq_disable();

        if (pending) {
                if (time_before(jiffies, end) && !need_resched() &&
                    --max_restart)
                        goto restart;
                wakeup_softirqd();
        }
}

```
Key points:
All pending softirqs are handled in a loop.
Execution is bounded by time and iteration limits.
Excess work is deferred to ksoftirqd to avoid long interrupt latency.

### Implementation

in tasklet_echo.c.
we recieve the user input string, store it into kernel buffer.
```c
static ssize_t tasklet_echo_write(struct file *filp,
                                  const char __user *ubuf,
                                  size_t len, loff_t *ppos)
{
        unsigned long flags;
        size_t n;

        /* 截斷到 BUF_SZ-1，留 '\0' */
        n = min(len, (size_t)BUF_SZ - 1);

        /*
         * copy_from_user 只能在 process context 做
         * (tasklet 不能做這件事)
         */
        if (copy_from_user(gbuf, ubuf, n))
                return -EFAULT;

        /* 去掉尾端換行更好看（可選） */
        while (n > 0 && (gbuf[n - 1] == '\n' || gbuf[n - 1] == '\r'))
                n--;

        spin_lock_irqsave(&gbuf_lock, flags);
        gbuf[n] = '\0';
        gbuf_len = n;
        spin_unlock_irqrestore(&gbuf_lock, flags);

        /* 在 write() 裡 schedule tasklet */
        tasklet_schedule(&tasklet_echo);

        return len; /* 回傳原始 len，符合一般 write 語意 */
}
```
in tasklet we print out the user intput string.
```c
static void tasklet_echo_handler(unsigned long data)
{
        unsigned long flags;
        char local[BUF_SZ];
        size_t len;

        spin_lock_irqsave(&gbuf_lock, flags);
        len = gbuf_len;
        if (len >= BUF_SZ)
                len = BUF_SZ - 1;
        memcpy(local, gbuf, len);
        local[len] = '\0';
        spin_unlock_irqrestore(&gbuf_lock, flags);

        pr_info("tasklet_echo: tasklet got: \"%s\" (len=%zu)\n", local, len);
}
```

result
```
~ # insmod tasklet_echo.ko
~ # ./write_tasklet_echo "william says hi"
[   67.109762] tasklet_echo: tasklet got: "william says hi" (len=15)
wrote 15 bytes: "william says hi"
```



