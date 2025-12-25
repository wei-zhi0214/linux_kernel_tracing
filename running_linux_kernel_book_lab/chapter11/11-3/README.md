#  Memory Management – Chapter 11 Labs
## **Lab: timer and kthread*

###  **Purpose**
1. Understand the timer and kthread mechanism. 
---

###  **Lab Requirement**
1.Using timer simulating interrput.If interrupt, wake up the thread to doing the work.
---

### Implementation

in timer_kthread_lab.c 
we use timer to wake up thread through waitqueue.
```c
static void timer_cb(struct timer_list *t)
{
	/* timer callback：softirq context，不可睡，只負責喚醒 */
	atomic_set(&ticked, 1);
	wake_up_interruptible(&wq);

	/* 重新排程下一次 timer */
	mod_timer(&t, jiffies + msecs_to_jiffies(period_ms));
}
```

and thread print out pid, name, jiffies, state, cpu
```c
static int worker_fn(void *arg)
{
        pr_info("11-3: kthread started pid=%d comm=%s\n",
                current->pid, current->comm);

        while (!kthread_should_stop()) {
                /* wait until timer fires or stop requested */
                wait_event_interruptible(wq,
                        atomic_read(&ticked) || kthread_should_stop());

                if (kthread_should_stop())
                        break;

                atomic_set(&ticked, 0);

                pr_info("11-3: wakeup! pid=%d comm=%s jiffies=%lu state=%c cpu=%d\n",
                        current->pid,
                        current->comm,
                        jiffies,
                        task_state_to_char(current),
                        raw_smp_processor_id());
        }

        pr_info("11-3: kthread stopping pid=%d\n", current->pid);
        return 0;
}
```

result
```
~ # insmod timer_kthread_lab.ko
[   11.147110] 11-3: module loaded (period_ms=500)
[   11.147644] 11-3: kthread started pid=61 comm=lab11_kthread
[   12.154102] 11-3: wakeup! pid=61 comm=lab11_kthread jiffies=4294679264 state=R cpu=0
[   12.660652] 11-3: wakeup! pid=61 comm=lab11_kthread jiffies=4294679771 state=R cpu=0
[   13.164601] 11-3: wakeup! pid=61 comm=lab11_kthread jiffies=4294680275 state=R cpu=0
[   13.668604] 11-3: wakeup! pid=61 comm=lab11_kthread jiffies=4294680779 state=R cpu=0
[   14.172534] 11-3: wakeup! pid=61 comm=lab11_kthread jiffies=4294681282 state=R cpu=0
[   14.676921] 11-3: wakeup! pid=61 comm=lab11_kthread jiffies=4294681787 state=R cpu=0
```

