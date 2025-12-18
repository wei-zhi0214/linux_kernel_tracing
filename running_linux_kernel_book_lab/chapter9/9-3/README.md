#  Memory Management – Chapter 8 Labs
## **Lab: alloc memory*

###  **Purpose**
1. understand and famillier with API of allocating memory. 

---

###  **Lab Requirement**

1. allocate page
Write kernel module, Use allocate_page function to get a page, and print out physical address, virtual address, fill this page with 0x55.
If use GFP_KERNEL or GFP_HIGHUSER_MOVABLE, what is different between them?
2.try to allocate max order memory
use __get_free_pages function to allocate max order memory, you can start with one order untill max order.

---
### allocate page

allocate page first will check out the free list, if there are, it will return it. if not, it will reclaim utill sucess, or fail to allocate.

```c
struct page *__alloc_pages(gfp_t gfp, unsigned int order, int preferred_nid,
							nodemask_t *nodemask)
{
	....
		page = get_page_from_freelist(alloc_gfp, order, alloc_flags, &ac);
	....
}
```

```c
static struct page *
get_page_from_freelist(gfp_t gfp_mask, unsigned int order, int alloc_flags,
						const struct alloc_context *ac)
{
	...
	for_next_zone_zonelist_nodemask(zone, z, ac->highest_zoneidx,
	ac->nodemask) {
		...
		//if there are not free memory
		if (!zone_watermark_fast(zone, order, mark,
			ac->highest_zoneidx, alloc_flags,
			gfp_mask)) {
			...
			// reclain 
			ret = node_reclaim(zone->zone_pgdat, gfp_mask, order);
			...
		}
try_this_zone:
		page = rmqueue(ac->preferred_zoneref->zone, zone, order,
			gfp_mask, alloc_flags, ac->migratetype);
	...

```
in get_page_from_freelist, it iterate all the zone to find free list, if there not, reclaim.

```c
static inline
struct page *rmqueue(struct zone *preferred_zone,
			struct zone *zone, unsigned int order,
			gfp_t gfp_flags, unsigned int alloc_flags,
			int migratetype)
{
	do {
		page = NULL;
		/*
		 * order-0 request can reach here when the pcplist is skipped
		 * due to non-CMA allocation context. HIGHATOMIC area is
		 * reserved for high-order atomic allocation, so order-0
		 * request should skip it.
		 */
		if (order > 0 && alloc_flags & ALLOC_HARDER) {
			page = __rmqueue_smallest(zone, order, MIGRATE_HIGHATOMIC);
			if (page)
				trace_mm_page_alloc_zone_locked(page, order, migratetype);
		}
		if (!page)
			page = __rmqueue(zone, order, migratetype, alloc_flags);
	} while (page && check_new_pages(page, order));
```
```c
static __always_inline
struct page *__rmqueue_smallest(struct zone *zone, unsigned int order,
						int migratetype)
{
	...
		for (current_order = order; current_order < MAX_ORDER; ++current_order) {
		area = &(zone->free_area[current_order]);
		page = get_page_from_free_area(area, migratetype);
		if (!page)
			continue;
		del_page_from_free_list(page, zone, current_order);
		expand(zone, page, order, current_order, migratetype);
		set_pcppage_migratetype(page, migratetype);
		return page;
	}

```
In __rmqueue_smallest, it will find smallest order of free memory. and then delete it from free list. if order of free memory is greater then ordfer of request. It will expand it put all page into next order.

Genernal say, when we allocate memory. If no memory, we can direct reclaim or let kswap to swap memory.
Reclaim will recycle some not crtical memory like cache, buffer, they don't use frequently. Direct Reclaim will more quickly get memory, but process will be block, because you do reclaim yourself, if we don't direct reclaim, we can wait kswap to swap memory to disk, that will be slow, but process won't be block, you can do other stall.

In mem_alloc_page.c
illustrate how to get a page and free it.
```c
    //get a page
    page = alloc_page(GFP_KERNEL);
    // kmap vaddr to physical addr
    vaddr = kmap_local_page(page);
    // umap
    kunmap_local(vaddr);
    //free memory
    __free_page(page);
```



### discussion gfp_mask
1.zone modifier
| Flag | Description |
|------|------------|
| __GFP_DMA | Use for DMA |
| __GFP_MOVABLE| Page can movable |

DMA memory is in specified memory location, and it need to allocate it in continous way.
Movable is saying, you can move this memory. Kernel can migration, reclaim, compact.
migration: copy the page to new page.
reclaim: if no memory, you can free it.
compact: move the memory get large continous memory.

2.mobility and placement modifiers
| Flag | Description |
|------|------------|
| __GFP_RECLAIMABLE | Can recliam |
| __GFP_WRITE| can writeback to get a memory |
Recalimable page is like cache, buffer.
Cache: We can load again from a disk. So we can drop it.
Buffer:If we drop it dosen't matter. then we can drop it.

Write:
Because need to wait writeback, latency may be huge. but easier to suceed.

3.watermark modifier
| Flag | Description |
|------|------------|
| __GFP_ATOMIC | Allocation is uninterrupt and unblocking |
other flag is the same, but bigger memory area.

4.reclaim modifiers

| Flag | Description |
|------|------------|
| __GFP_IO| use io to get memroy like swap/writeback |
| __GFP_FS| allow to use file system call.|
|__GFP_DIRECT_RECLAIM| can direct reclaim|
|__GFP_KSWAPD_RECLAIM| if reach low watermark wake up kswap|

result

```
[   10.979270] alloc_page: page=ffffea0000136dc0
[   10.979531] alloc_page: phys=0x4db7000
[   10.979621] alloc_page: virt=ffff888004db7000
[   10.979738] alloc_page: done
```

### get free page
In mem_max_contig.c
result
```
[   66.104591] max_contig: ok order=0 pages=1 bytes=4096 vaddr=ffff888004fd6000
[   66.105019] max_contig: ok order=1 pages=2 bytes=8192 vaddr=ffff88800475a000
[   66.105153] max_contig: ok order=2 pages=4 bytes=16384 vaddr=ffff8880044e0000
[   66.105296] max_contig: ok order=3 pages=8 bytes=32768 vaddr=ffff888004db0000
[   66.105403] max_contig: ok order=4 pages=16 bytes=65536 vaddr=ffff888004f60000
[   66.105823] max_contig: ok order=5 pages=32 bytes=131072 vaddr=ffff888004d80000
[   66.105977] max_contig: ok order=6 pages=64 bytes=262144 vaddr=ffff888004f00000
[   66.106090] max_contig: ok order=7 pages=128 bytes=524288 vaddr=ffff888004e00000
[   66.106416] max_contig: ok order=8 pages=256 bytes=1048576 vaddr=ffff888004e00000
[   66.106610] max_contig: ok order=9 pages=512 bytes=2097152 vaddr=ffff888004a00000
[   66.106890] max_contig: ok order=10 pages=1024 bytes=4194304 vaddr=ffff888005000000
[   66.107198] max_contig: max_ok_order=10 (pages=1024, bytes=4194304)
```

###
