#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include "cpu/reg.h"
#include "memory/tlb.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	int first_id = readCache(addr);
	uint32_t offset = addr&(L1_CACHE_BLOCK_SIZE - 1);
	uint8_t temp[2*BURST_LEN];

	if (offset + len > L1_CACHE_BLOCK_SIZE)
	{
		int second_id = readCache(addr + L1_CACHE_BLOCK_SIZE - offset);
		memcpy(temp, l1_cache[first_id].data + offset, L1_CACHE_BLOCK_SIZE - offset);
		memcpy(temp + L1_CACHE_BLOCK_SIZE - offset, l1_cache[second_id].data, len - L1_CACHE_BLOCK_SIZE + offset);
	} else
	{
		memcpy(temp, l1_cache[first_id].data + offset, len);
	}

	int zero = 0;
	uint32_t ans = unalign_rw(temp+zero, 4) & (~0u >> ((4 - len) << 3));
	
	return ans;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	writeCache(addr, len, data);
}

hwaddr_t page_translate(lnaddr_t addr, size_t len) {
	if(cpu.cr0.protect_enable && cpu.cr0.paging) {
		hwaddr_t tmpaddr;
		if ((tmpaddr = readTLB(addr & 0xfffff000)) != -1)
		{
			return (tmpaddr << 12) + (addr & 0xfff);
		}
		
		PageEntry dir, page;
		uint32_t dir_offset = addr>>22;
		uint32_t page_offset = ((addr>>12) & 0x3ff);
		uint32_t offset = addr & 0xfff;
		dir.val = hwaddr_read((cpu.cr3.page_directory_base << 12) + (dir_offset << 2), 4);
		Assert(dir.p, "Invalid Page!");
		page.val = hwaddr_read((dir.base << 12) + (page_offset << 2), 4);
		Assert(page.p, "Invalid Page!");
		writeTLB(addr & 0xfffff000, page.base);
		return (page.base << 12)+offset;
	} else {
		return addr;
	}
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	assert(len == 1 || len == 2 || len == 4);
	hwaddr_t hwaddr = page_translate(addr, len);
	return hwaddr_read(hwaddr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	assert(len == 1 || len == 2 || len == 4);
	hwaddr_t hwaddr = page_translate(addr, len);
	hwaddr_write(hwaddr, len, data);
}

void loadSregCache(uint8_t sreg) {
	uint32_t gdt = cpu.gdtr.base_addr; // base
	gdt += cpu.sr[sreg].index << 3; // offset
	SegmentDestriptor sdp;
	sdp.first = lnaddr_read(gdt, 4);
	sdp.second = lnaddr_read(gdt+4, 4);
	uint32_t base = (((uint32_t)sdp.base2) << 16) | sdp.base1 | (((uint32_t)sdp.base3) << 24);
	uint32_t limit = (((uint32_t)sdp.limit2) << 16) | sdp.limit1;
	if(sdp.g) limit<<=12;
	cpu.sr[sreg].cache.limit = limit;
	cpu.sr[sreg].cache.base = base;
}

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg) {
	if (cpu.cr0.protect_enable) // protected mode
	{
		Assert(addr+len < cpu.sr[sreg].cache.limit, "Segmentation Fault.");
		return addr+cpu.sr[sreg].cache.base;
	} else
	{
		return addr;
	}
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	lnaddr_write(lnaddr, len, data);
}

