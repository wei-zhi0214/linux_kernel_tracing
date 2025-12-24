#  Memory Management – Chapter 11 Labs
## **Lab: tasklet*

###  **Purpose**
1. Understand the workqueue mechanism in Linux.
---

###  **Lab Requirement**
1. Write a kernel module that:
   Initializes a workqueue.
   Schedules the  workqueue from a write() file operation.
   Prints the user-provided string inside the workqueue callback.
2. Write a user-space application to test the module.
---
### Background

What is a workqueue?

A workqueue mechanism is like thread pool.
Shortly say thread pool, if the system have lots of thread, loading will be huge.Manange thread will cost a lot of work.So we will want to limit total number of thread in system.
Another thing is if we create thread, we need to malloc memory, prepare all task structure information, if you requet coming frequently, system will do create thread a lot.So in this scenario, we create some thread in thread pool, we don't destory the thread. if next request comming, we give the work to this thread to doing the work to avoid overhead of creating a thread.
Workqueue basiclly is create a thread, so the work is in process context. we don't have to worry if the work is heavy will increase lantancy of other task. We can do memory stuff, heavy computation, sleep, blocking in workqueue.


### Implementation

in wq_echo.c
we recieve the user input string, and malloc memory to store it to avoid use the same buffer next time write will overwite it. We create queue like structure to store message. 
```c
#define MSG_MAX  256

struct msg {
        struct list_head node;
        size_t len;
        char data[MSG_MAX];
};
static LIST_HEAD(msg_q);
static spinlock_t q_lock;

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
```

and worker does is copy the message queue to local, and then print it out. Because print out will take a lot of time, so we copy message to local memory, if we write again the message qeueu can be modify, won't be blocking if we use message queue to print out. Important thing, we can do this memory operation because we are in process content, we won't worry about blocking or latency.
```c
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
```

result
```
~ # insmod wq_echo.ko
[   60.342694] wq_echo: "william wq test" (len=15)
wrote 15 bytes: "william wq test"
```

