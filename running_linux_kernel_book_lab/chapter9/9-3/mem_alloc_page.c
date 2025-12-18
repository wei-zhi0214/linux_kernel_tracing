// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/highmem.h>
#include <linux/io.h>

static int __init mem_alloc_page_init(void)
{
    struct page *page;
    void *vaddr;
    phys_addr_t paddr;

    /* 1) 分配一個物理頁面 */
    page = alloc_page(GFP_KERNEL);
    if (!page) {
        pr_err("alloc_page failed\n");
        return -ENOMEM;
    }

    /* 2) 物理位址（PFN -> PA） */
    paddr = page_to_phys(page);

    /*
     * 3) kernel 虛擬位址
     *    在某些平台/情況 page 可能是 highmem，不能直接 page_address。
     *    用 kmap_local_page() 是最通用、安全的方式。
     */
    vaddr = kmap_local_page(page);

    pr_info("alloc_page: page=%px\n", page);
    pr_info("alloc_page: phys=0x%llx\n", (unsigned long long)paddr);
    pr_info("alloc_page: virt=%px\n", vaddr);

    /* 4) 填 0x55 */
    memset(vaddr, 0x55, PAGE_SIZE);

    kunmap_local(vaddr);

    /* 釋放 */
    __free_page(page);

    pr_info("alloc_page: done\n");
    return 0;
}

static void __exit mem_alloc_page_exit(void)
{
    pr_info("alloc_page: exit\n");
}

module_init(mem_alloc_page_init);
module_exit(mem_alloc_page_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("william");
MODULE_DESCRIPTION("Lab 9-3(1): alloc_page print PA/VA and fill 0x55");

