#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include <string.h>
#include "nemu.h"


uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

hwaddr_t page_translate(lnaddr_t addr){
	return addr;
}

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	//return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	uint32_t offset = addr & (Cache_L1_Block_Size - 1);
	uint8_t temp[BURST_LEN << 1];

	int start_address = read_cache1(addr);
	

	if(offset + len > Cache_L1_Block_Size){
		memcpy(temp, cache1[start_address].data + offset, Cache_L1_Block_Size - offset);
		int next_address = read_cache1(addr + Cache_L1_Block_Size - offset);
		memcpy(temp + (Cache_L1_Block_Size - offset), cache1[next_address].data, len - (Cache_L1_Block_Size - offset));
	}else{
		memcpy(temp, cache1[start_address].data + offset, len);
	}

	int zero = 0;
	uint32_t ans = unalign_rw(temp + zero, 4) & (~0u >> ((4 - len) << 3));
	return ans;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	//dram_write(addr, len, data);
	write_cache1(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	// return hwaddr_read(addr, len);
	assert(len == 1 || len == 2 || len ==4 );
	if(addr){
		assert(0);
	}else{
		hwaddr_t hwaddr = page_translate(addr);
		return hwaddr_read(hwaddr, len);
	}
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	// hwaddr_write(addr, len, data);

	assert(len == 1 || len == 2 || len ==4 );
	if(addr){
		assert(0);
	}else{
		hwaddr_t hwaddr = page_translate(addr);
		hwaddr_write(hwaddr, len, data);
	}
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, current_sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr,len,current_sreg);
	lnaddr_write(lnaddr, len, data);
}

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg){
	if(cpu.CR0.protect_enable == 0){
		return addr;
	}else{
		return cpu.segment_reg[sreg].base+addr;
	}
}