#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/cpumask.h>
#include <linux/printk.h>
#include <linux/preempt.h>

#define DRV_NAME "kthread_lab"

static int loops = 3;          // 每條 thread 要跑幾圈
module_param(loops, int, 0644);
MODULE_PARM_DESC(loops, "number of loops to run in each kthread");

static int interval_ms = 300;  // 每圈間隔 (ms)
module_param(interval_ms, int, 0644);
MODULE_PARM_DESC(interval_ms, "sleep interval (ms) between prints");

static struct task_struct *workers[NR_CPUS];

static int worker_fn(void *data)
{
	int cpu = smp_processor_id();
	int i;

	/* 若不是用 kthread_create_on_cpu()，也可在這裡用 kthread_bind() 綁定 CPU */

	pr_info("[%s] start on CPU %d: pid=%d prio=%d static_prio=%d nice=%d policy=%d rt_prio=%d preempt_cnt=%u\n",
		DRV_NAME, cpu, current->pid, current->prio, current->static_prio,
		task_nice(current), current->policy, current->rt_priority,
		preempt_count());

	/* ---- 範例：列印部分暫存器（依架構條件編譯） ---- */
#if defined(CONFIG_X86_64)
	{
		unsigned long rbp, rsp;
		asm volatile("mov %%rbp,%0" : "=r"(rbp));
		asm volatile("mov %%rsp,%0" : "=r"(rsp));
		pr_info("[%s][CPU%d] regs(x86_64): RBP=%px RSP=%px\n",
			DRV_NAME, cpu, (void *)rbp, (void *)rsp);
	}
#elif defined(CONFIG_ARM64)
	{
		unsigned long x29, sp, currentel;
		asm volatile("mov %0, x29" : "=r"(x29));
		asm volatile("mov %0, sp"  : "=r"(sp));
		asm volatile("mrs %0, CurrentEL" : "=r"(currentel));
		pr_info("[%s][CPU%d] regs(arm64): X29(FP)=%px SP=%px CurrentEL=%lx\n",
			DRV_NAME, cpu, (void *)x29, (void *)sp, currentel);
	}
#else
	pr_info("[%s][CPU%d] reg dump not implemented on this arch\n",
		DRV_NAME, cpu);
#endif

	for (i = 0; i < loops && !kthread_should_stop(); i++) {
		/* 顯示每圈的基本狀態變化 */
		pr_info("[%s][CPU%d][loop %d/%d] pid=%d prio=%d nice=%d policy=%d preempt_cnt=%u\n",
			DRV_NAME, cpu, i + 1, loops, current->pid,
			current->prio, task_nice(current), current->policy,
			preempt_count());

		msleep(interval_ms);
	}

	pr_info("[%s] stop on CPU %d\n", DRV_NAME, cpu);
	return 0;
}

static int __init kthread_lab_init(void)
{
	int cpu;

	pr_info("[%s] init: creating one kthread per online CPU\n", DRV_NAME);

	for_each_online_cpu(cpu) {
		struct task_struct *t;

		t = kthread_create(worker_fn, NULL, DRV_NAME "/%u", cpu); // 這個可以用 %u
		if (IS_ERR(t)) {
			pr_err("[%s] create on CPU %d failed: %ld\n",
					DRV_NAME, cpu, PTR_ERR(t));
			workers[cpu] = NULL;
			continue;
		}
		kthread_bind(t, cpu);   // 綁定到指定 CPU
		workers[cpu] = t;
		wake_up_process(t);
	}
	return 0;
}

static void __exit kthread_lab_exit(void)
{
	int cpu;

	for_each_online_cpu(cpu) {
		if (workers[cpu]) {
			kthread_stop(workers[cpu]);
			workers[cpu] = NULL;
		}
	}
	pr_info("[%s] exit\n", DRV_NAME);
}

module_init(kthread_lab_init);
module_exit(kthread_lab_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Lab: create a kernel thread on each CPU and dump state/priority");
MODULE_LICENSE("GPL");

