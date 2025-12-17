// drivers/misc/memstat_lab.c
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/page-flags.h>
#include <linux/bitops.h>

#include <linux/memblock.h>   // ✅ for_each_mem_pfn_range
#include <linux/numa.h>       // numa_valid_node (optional)
#include <linux/mmzone.h>     // MAX_NUMNODES

static inline bool page_has_flag(struct page *page, enum pageflags flag)
{
    return test_bit(flag, &page->flags);
}

static inline unsigned int buddy_order_safe(struct page *page)
{
    unsigned int order = (unsigned int)page_private(page);
    if (order > 20) order = 0;
    return order;
}

static int __init memstat_lab_init(void)
{
    unsigned long long total_pages_seen = 0;

    unsigned long long free_pages = 0;
    unsigned long long reserved_pages = 0;
    unsigned long long swapcache_pages = 0;
    unsigned long long slab_pages = 0;
    unsigned long long dirty_pages = 0;
    unsigned long long active_pages = 0;
    unsigned long long writeback_pages = 0;

    unsigned long start_pfn, end_pfn;
    int i, nid;

    /*
     * Linux 6.17 常見用法（memblock 範圍轉 PFN 範圍）
     * for_each_mem_pfn_range(i, MAX_NUMNODES, &start_pfn, &end_pfn, &nid)
     */
    for_each_mem_pfn_range(i, MAX_NUMNODES, &start_pfn, &end_pfn, &nid) {
        unsigned long pfn;

        if (!numa_valid_node(nid))
            continue;

        for (pfn = start_pfn; pfn < end_pfn; pfn++) {
            struct page *page;

            if (!pfn_valid(pfn))
                continue;

            page = pfn_to_page(pfn);
            total_pages_seen++;

#ifdef PageBuddy
            if (PageBuddy(page)) {
                free_pages += (1ULL << buddy_order_safe(page));
                continue;
            }
#endif

            if (page_has_flag(page, PG_reserved))
                reserved_pages++;

#ifdef PG_swapcache
            if (page_has_flag(page, PG_swapcache))
                swapcache_pages++;
#endif
#ifdef PG_slab
            if (page_has_flag(page, PG_slab))
                slab_pages++;
#endif
            if (page_has_flag(page, PG_dirty))
                dirty_pages++;
#ifdef PG_active
            if (page_has_flag(page, PG_active))
                active_pages++;
#endif
            if (page_has_flag(page, PG_writeback))
                writeback_pages++;
        }
    }

    pr_info("memstat_lab: total_pages_seen=%llu\n", total_pages_seen);
    pr_info("memstat_lab: free_pages(buddy)=%llu\n", free_pages);
    pr_info("memstat_lab: reserved_pages=%llu\n", reserved_pages);
    pr_info("memstat_lab: swapcache_pages=%llu\n", swapcache_pages);
    pr_info("memstat_lab: slab_pages=%llu\n", slab_pages);
    pr_info("memstat_lab: dirty_pages=%llu\n", dirty_pages);
    pr_info("memstat_lab: active_pages=%llu\n", active_pages);
    pr_info("memstat_lab: writeback_pages=%llu\n", writeback_pages);

    return 0;
}

late_initcall(memstat_lab_init);
