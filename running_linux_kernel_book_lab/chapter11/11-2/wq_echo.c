// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/list.h>

#define DEV_NAME "wq_echo"
#define MSG_MAX  256

struct msg {
	struct list_head node;
	size_t len;
	char data[MSG_MAX];
};

static struct workqueue_struct *wq;
static struct work_struct work;

static LIST_HEAD(msg_q);
static spinlock_t q_lock;

static void wq_echo_worker(struct work_struct *work)
{
	LIST_HEAD(local);
	unsigned long flags;
	struct msg *m, *tmp;

	/*
	 * workqueue 是 process context，可以睡。
	 * 但我們依然用 spinlock 保護 queue，然後把 queue 搬到 local 再處理，
	 * 讓 critical section 很短。
	 */
	spin_lock_irqsave(&q_lock, flags);
	list_splice_init(&msg_q, &local);
	spin_unlock_irqrestore(&q_lock, flags);

	/* 處理所有待處理訊息 */
	list_for_each_entry_safe(m, tmp, &local, node) {
		list_del(&m->node);
		pr_info("wq_echo: \"%s\" (len=%zu)\n", m->data, m->len);
		kfree(m);
	}
}

static ssize_t wq_echo_write(struct file *filp,
			     const char __user *ubuf,
			     size_t len, loff_t *ppos)
{
	struct msg *m;
	size_t n;

	/* 每次 write 都分配一個 message，避免覆蓋 */
	m = kzalloc(sizeof(*m), GFP_KERNEL);
	if (!m)
		return -ENOMEM;

	n = min(len, (size_t)MSG_MAX - 1);

	if (copy_from_user(m->data, ubuf, n)) {
		kfree(m);
		return -EFAULT;
	}

	/* 去掉尾端換行（可選） */
	while (n > 0 && (m->data[n - 1] == '\n' || m->data[n - 1] == '\r'))
		n--;

	m->data[n] = '\0';
	m->len = n;

	/* 入列 */
	spin_lock(&q_lock);
	list_add_tail(&m->node, &msg_q);
	spin_unlock(&q_lock);

	/* 觸發 worker（多次 queue_work 沒關係，work_struct 同時只會跑一次） */
	queue_work(wq, &work);

	return len;
}

static const struct file_operations wq_echo_fops = {
	.owner = THIS_MODULE,
	.write = wq_echo_write,
};

static struct miscdevice wq_echo_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEV_NAME,
	.fops  = &wq_echo_fops,
	.mode  = 0666,
};

static int __init wq_echo_init(void)
{
	int ret;

	spin_lock_init(&q_lock);
	INIT_WORK(&work, wq_echo_worker);

	/*
	 * 自建 workqueue（更像真實 driver）
	 * WQ_UNBOUND：不綁 CPU，簡單好用
	 */
	wq = alloc_workqueue("wq_echo_wq", WQ_UNBOUND, 1);
	if (!wq)
		return -ENOMEM;

	ret = misc_register(&wq_echo_misc);
	if (ret) {
		destroy_workqueue(wq);
		return ret;
	}

	pr_info("wq_echo: loaded. /dev/%s\n", DEV_NAME);
	return 0;
}

static void __exit wq_echo_exit(void)
{
	struct msg *m, *tmp;
	unsigned long flags;

	/*
	 * 先停止接新工作（卸載時）
	 * flush_workqueue 只 flush 我們自己的 queue，不會觸發 6.17 的 warning
	 */
	if (wq) {
		flush_workqueue(wq);
		destroy_workqueue(wq);
	}

	misc_deregister(&wq_echo_misc);

	/* 清掉殘留 message（理論上 flush 後應該為空，但保險） */
	spin_lock_irqsave(&q_lock, flags);
	list_for_each_entry_safe(m, tmp, &msg_q, node) {
		list_del(&m->node);
		kfree(m);
	}
	spin_unlock_irqrestore(&q_lock, flags);

	pr_info("wq_echo: unloaded\n");
}

module_init(wq_echo_init);
module_exit(wq_echo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("william");
MODULE_DESCRIPTION("Lab 11-2: workqueue driver (realistic) with message queue");

