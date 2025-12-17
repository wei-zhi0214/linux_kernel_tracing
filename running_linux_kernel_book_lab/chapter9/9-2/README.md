#  Memory Management – Chapter 8 Labs
## **Lab: obtain physical memory information**

###  **Purpose**
1. understand and famillier with meth of physical memory management, for example, usage of page structrure and flags. 

---

###  **Lab Requirement**

1. Every physical page one to one mapping to page structure. Kernel allocate a page structure for every physical page, and store it in global array, mem_map[]. Writing kernal module. iterate all mem_map[] array to statistics free page, reserved page, swap page, slab page, dirty page, active page, writeback page.

---
### Page

Kernel use mem_map[] array store page structure to descrip the physical page.

```
struct page {
	unsigned long flags;
	....
}
```

```
enum pageflags {

PG_locked, /* 页面已经上锁，不要访问 */

PG_error, // 表示页面发生了I/O错误

PG_referenced, // 此标志位用来实现LRU算法中第二次机会法

PG_uptodate, // 标示页面内容是有效的，当该页面上读操作完成之后，设置该标志位

PG_dirty, // 表示页面内容被修改过，为脏页

PG_lru, // 表示该页在LRU链表中

PG_active, // 表示该页在活跃LRU链表中

PG_slab, // 表示页属于由slab分配器创建的slab

PG_owner_priv_1, /* 页面的所有者使用，如果是pagecache页面，文件系统可能使用*/

PG_arch_1, // 与体系结构相关的页面状态位

PG_reserved, // 表示页不可被换出

PG_private, /* 表示该页是有效的，当page->private包含有效值时会设置此标志位，如果是pagecache，那么包含一个文件系统相关的数据信息 */

PG_private_2, /* 如果是pagecache，可能包含 FS aux data */

PG_writeback, /* 页面正在回写 */

#ifdef CONFIG_PAGEFLAGS_EXTENDED

PG_head, /* A head page */

PG_tail, /* A tail page */

#else

PG_compound, /* 一个混合页面 */

#endif

PG_swapcache, /* 交换页面*/

PG_mappedtodisk, /* 在磁盘中分配blocks */

PG_reclaim, /* 立刻要被回收 */

PG_swapbacked, /* 页面是不可回收的 */

PG_unevictable, /* Page is "unevictable" */

#ifdef CONFIG_MMU

PG_mlocked, // VMA处于mlocked状态

#endif

#ifdef CONFIG_ARCH_USES_PG_UNCACHED

PG_uncached, /* Page has been mapped as uncached */

#endif

#ifdef CONFIG_MEMORY_FAILURE

PG_hwpoison, /* hardware poisoned page. Don't touch */

#endif

#ifdef CONFIG_TRANSPARENT_HUGEPAGE

PG_compound_lock,

#endif

__NR_PAGEFLAGS,

/* Filesystems */

PG_checked = PG_owner_priv_1,

/* Two page bits are conscripted by FS-Cache to maintain local caching

* state. These bits are set on pages belonging to the netfs's inodes

* when those inodes are being locally cached.

*/

PG_fscache = PG_private_2, /* page backed by cache */

/* XEN */

/* Pinned in Xen as a read-only pagetable page. */

PG_pinned = PG_owner_priv_1,

/* Pinned as part of domain save (see xen_mm_pin_all()). */

PG_savepinned = PG_dirty,

/* Has a grant mapping of another (foreign) domain's page. */

PG_foreign = PG_owner_priv_1,

/* SLOB */

PG_slob_free = PG_private,

};
```
we use flags of page to count how many page is free page, reserved page, swapcache page, slab page, dirty page, active page, writeback page.
Iterate all mem_map[] to get every pages.
Because global mem_map array is not efficent, it change data structure to every node. The other way to get the page is using memblock system. It will get ram address from dts. Memoryblcok structure have start pfn and end pfn. We iterate all the ram memory. And use pfn to get the page.
Use fuction for_each_mem_pfn_range() to get pfn
```c
    for_each_mem_pfn_range(i, MAX_NUMNODES, &start_pfn, &end_pfn, &nid) {
```

and use pfn_to_page() to get page.
```c
    page = pfn_to_page(pfn);
```

finallly, use flags to count how many each type of page in ram. We use macros, this macros just check out the bit.
```c
static inline bool page_has_flag(struct page *page, enum pageflags flag)
{
    return test_bit(flag, &page->flags);
}

### result
```bash
[    2.630155] memstat_lab: total_pages_seen=262014
[    2.630299] memstat_lab: free_pages(buddy)=0
[    2.630379] memstat_lab: reserved_pages=15462
[    2.630450] memstat_lab: swapcache_pages=0
[    2.630777] memstat_lab: slab_pages=0
[    2.630849] memstat_lab: dirty_pages=960
[    2.630915] memstat_lab: active_pages=0
[    2.630976] memstat_lab: writeback_pages=61
```

