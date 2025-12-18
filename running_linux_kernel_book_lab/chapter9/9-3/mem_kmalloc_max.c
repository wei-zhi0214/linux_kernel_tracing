// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

static int __init mem_kmalloc_max_init(void)
{
    size_t sz;
    void *p = NULL;

    /*
     * 從 1KB 開始倍增到 8MB（你也可以調上限）
     * GFP_ATOMIC：不可睡眠，容易失敗，符合題目精神
     */
    for (sz = 1024; sz <= (8UL << 20); sz <<= 1) {
        p = kmalloc(sz, GFP_ATOMIC);
        if (!p) {
            pr_info("kmalloc_max: FAIL at %zu bytes\n", sz);
            break;
        }
        pr_info("kmalloc_max: OK   at %zu bytes, ptr=%px\n", sz, p);
        kfree(p);
        p = NULL;
    }

    return 0;
}

static void __exit mem_kmalloc_max_exit(void)
{
    pr_info("kmalloc_max: exit\n");
}

module_init(mem_kmalloc_max_init);
module_exit(mem_kmalloc_max_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("william");
MODULE_DESCRIPTION("Lab 9-3: probe maximum kmalloc size with GFP_ATOMIC");

