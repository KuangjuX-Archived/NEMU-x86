#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include <string.h>
#include "nemu.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	uint32_t offset = addr & BURST_MASK;
	uint8_t temp[BURST_LEN << 1];

	int start_address = read_cache1(addr);
	

	if(offset + len > Cache_L1_Block_Size){
		//Log();
		memcpy(temp, cache1[start_address].data + offset, Cache_L1_Block_Size - offset);
		//Log();
		int next_address = read_cache1(addr + Cache_L1_Block_Size - offset);
		memcpy(temp + (Cache_L1_Block_Size - offset), cache1[next_address].data, len - (Cache_L1_Block_Size - offset));
	}else{
		memcpy(temp, cache1[start_address].data + offset, len);
	}

	int tmp = 0;
	uint32_t ans = unalign_rw(temp + tmp, 4) & (~0u >> ((4 - len) << 3));
	return ans;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	//dram_write(addr, len, data);
	write_cache1(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(addr, len, data);
}

