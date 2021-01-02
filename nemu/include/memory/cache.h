#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

#define L1_CACHE_BLOCK_SIZE_BIT 6
#define L1_CACHE_WAY_BIT 3
#define L1_CACHE_SET_BIT 7

#define L1_CACHE_BLOCK_SIZE (1<<L1_CACHE_BLOCK_SIZE_BIT)
#define L1_CACHE_WAY_SIZE (1<<L1_CACHE_WAY_BIT)
#define L1_CACHE_SET_SIZE (1<<L1_CACHE_SET_BIT)

#define L2_CACHE_BLOCK_SIZE_BIT 6
#define L2_CACHE_WAY_BIT 4
#define L2_CACHE_SET_BIT 12

#define L2_CACHE_BLOCK_SIZE (1<<L2_CACHE_BLOCK_SIZE_BIT)
#define L2_CACHE_WAY_SIZE (1<<L2_CACHE_WAY_BIT)
#define L2_CACHE_SET_SIZE (1<<L2_CACHE_SET_BIT)

uint64_t MEMORY_TIME;

typedef struct {
    uint8_t data[L1_CACHE_BLOCK_SIZE];
    uint32_t tag;
    bool valid;
} L1CacheBlock;

typedef struct {
    uint8_t data[L2_CACHE_BLOCK_SIZE];
    uint32_t tag;
    bool valid, dirty;
} L2CacheBlock;

L1CacheBlock l1_cache[L1_CACHE_SET_SIZE * L1_CACHE_WAY_SIZE];
L2CacheBlock l2_cache[L2_CACHE_SET_SIZE * L2_CACHE_WAY_SIZE];

void resetCache();
int readCache(hwaddr_t addr);
void writeCache(hwaddr_t addr, size_t len, uint32_t data);
void addMemoryTime(uint32_t t);

int readCache2(hwaddr_t addr);
void writeCache2(hwaddr_t addr, size_t len, uint32_t data);

#endif