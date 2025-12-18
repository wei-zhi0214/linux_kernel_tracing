// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/highmem.h>

static int __init mem_max_contig_init(void)
{
    int order;
    unsigned long addr;
    int max_ok = -1;

    /*
     * 注意：GFP_ATOMIC 不能睡眠、不能做重 reclaim，
     * 可用資源更少，所以可分配的最大連續區通常更小。
     */
    for (order = 0; order <= NR_PAGE_ORDERS - 1; order++) {
        addr = __get_free_pages(GFP_ATOMIC, order);
        if (!addr) {
            pr_info("max_contig: fail at order=%d (pages=%lu, bytes=%lu)\n",
                    order, 1UL << order, (1UL << order) * PAGE_SIZE);
            break;
        }

        pr_info("max_contig: ok order=%d pages=%lu bytes=%lu vaddr=%px\n",
                order, 1UL << order, (1UL << order) * PAGE_SIZE, (void *)addr);

        free_pages(addr, order);
        max_ok = order;
    }

    if (max_ok >= 0)
        pr_info("max_contig: max_ok_order=%d (pages=%lu, bytes=%lu)\n",
                max_ok, 1UL << max_ok, (1UL << max_ok) * PAGE_SIZE);

    return 0;
}

static void __exit mem_max_contig_exit(void)
{
    pr_info("max_contig: exit\n");
}

module_init(mem_max_contig_init);
module_exit(mem_max_contig_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("william");
MODULE_DESCRIPTION("Lab 9-3(2): find max contiguous pages with __get_free_pages(GFP_ATOMIC)");

