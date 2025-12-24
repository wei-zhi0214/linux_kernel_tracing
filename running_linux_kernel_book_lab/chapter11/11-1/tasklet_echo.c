// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#define DEV_NAME "tasklet_echo"
#define BUF_SZ   256

static char gbuf[BUF_SZ];
static size_t gbuf_len;
static spinlock_t gbuf_lock;

/* tasklet handler: softirq context, MUST NOT sleep */
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

static struct tasklet_struct tasklet_echo;

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

static const struct file_operations tasklet_echo_fops = {
	.owner = THIS_MODULE,
	.write = tasklet_echo_write,
};

static struct miscdevice tasklet_echo_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEV_NAME,
	.fops  = &tasklet_echo_fops,
	.mode  = 0666,
};

static int __init tasklet_echo_init(void)
{
	spin_lock_init(&gbuf_lock);
	gbuf_len = 0;

	if (misc_register(&tasklet_echo_misc))
		return -ENODEV;
	tasklet_init(&tasklet_echo, tasklet_echo_handler, 0);
	pr_info("tasklet_echo: loaded. /dev/%s\n", DEV_NAME);
	return 0;
}

static void __exit tasklet_echo_exit(void)
{
	/* 確保 tasklet 不再跑 */
	tasklet_kill(&tasklet_echo);
	misc_deregister(&tasklet_echo_misc);
	pr_info("tasklet_echo: unloaded\n");
}

module_init(tasklet_echo_init);
module_exit(tasklet_echo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("william");
MODULE_DESCRIPTION("Lab 11-1: tasklet scheduled from write(), prints user string in tasklet handler");

