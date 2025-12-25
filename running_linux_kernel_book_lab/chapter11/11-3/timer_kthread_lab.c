// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/atomic.h>
#include <linux/sched.h>        // current
#include <linux/sched/signal.h> // task_state_to_char()
#include <linux/smp.h>          // raw_smp_processor_id()

static int period_ms = 500;
module_param(period_ms, int, 0444);
MODULE_PARM_DESC(period_ms, "Timer period in ms");

static struct task_struct *worker;
static wait_queue_head_t wq;
static atomic_t ticked = ATOMIC_INIT(0);

/* IMPORTANT: must be an object, not pointer */
static struct timer_list gtimer;

/* timer callback: softirq context, must not sleep */
static void timer_cb(struct timer_list *unused)
{
	atomic_set(&ticked, 1);
	wake_up_interruptible(&wq);

	/* re-arm timer */
	mod_timer(&gtimer, jiffies + msecs_to_jiffies(period_ms));
}

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

static int __init timer_kthread_init(void)
{
	int ret;

	init_waitqueue_head(&wq);

	worker = kthread_run(worker_fn, NULL, "lab11_kthread");
	if (IS_ERR(worker)) {
		ret = PTR_ERR(worker);
		pr_err("11-3: kthread_run failed: %d\n", ret);
		return ret;
	}

	/* init timer */
	timer_setup(&gtimer, timer_cb, 0);
	mod_timer(&gtimer, jiffies + msecs_to_jiffies(period_ms));

	pr_info("11-3: module loaded (period_ms=%d)\n", period_ms);
	return 0;
}

static void __exit timer_kthread_exit(void)
{
	pr_info("11-3: exit begin\n");

	/* stop timer (sync ensures callback is not running) */
	timer_shutdown_sync(&gtimer);

	if (worker) {
		/* wake it so it won't hang in wait_event */
		wake_up_interruptible(&wq);
		kthread_stop(worker);
	}

	pr_info("11-3: module unloaded\n");
}

module_init(timer_kthread_init);
module_exit(timer_kthread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("william");
MODULE_DESCRIPTION("Lab 11-3: timer wakes up a kthread; kthread prints pid/jiffies/state");

