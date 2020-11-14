#include "common.h"
#include "memory/cache.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "burst.h"

void public_ddr3_read(hwaddr_t addr, void* data);
void public_ddr3_write(hwaddr_t addr, void* data, uint8_t *mask);

void dram_write(hwaddr_t addr, size_t len, uint32_t data);

void init_cache(){
    //initialize cache L1
    int i;
    for (i = 0;i < Cache_L1_Size / Cache_L1_Block_Size;i++){
        cache1[i].valid = 0;
    }

    //initialize cache L2
    for (i = 0;i < Cache_L2_Size / Cache_L2_Block_Size;i++){
        cache2[i].valid = 0;
        cache2[i].dirty = 0;
    }
    test_time = 0;
}

int read_cache1(hwaddr_t address){
    
    uint32_t group_id = (address >> Cache_L1_Block_Bit) & (Cache_L1_Group_Size - 1);
    uint32_t tag_id = (address >> (Cache_L1_Block_Bit + Cache_L1_Group_Bit));

    int i, group_position;
    group_position = group_id * Cache_L1_Way_Size;
    
    
    for(i = group_position; i < (group_position + Cache_L1_Way_Size); i++){
        if(cache1[i].valid == 1 && cache1[i].tag == tag_id){
            //HIT Cache_1

#ifndef TEST
    test_time += 2;
 #endif                  

            return i;
        }
    }



    //Fail to hit cache , replace with random algorithm
    //read address from cache2
    int replace = read_cache2(address);
    srand((unsigned int)(time(NULL)));
    i = group_position + (rand() % Cache_L1_Way_Size);

    
    memcpy(cache1[i].data,cache2[replace].data,Cache_L1_Block_Size);

    cache1[i].valid = 1;
    cache1[i].tag = tag_id;
    
    return i;


}


int read_cache2(hwaddr_t address){
    uint32_t group_id = (address >> Cache_L2_Block_Bit) & (Cache_L2_Group_Size-1);
    uint32_t tag = address >> (Cache_L2_Block_Bit + Cache_L2_Group_Bit);
    //set start position of copying address
    uint32_t block_start = (address >> Cache_L2_Block_Bit) << Cache_L2_Block_Bit;

    int i,group_position;
    group_position = group_id * Cache_L2_Way_Size;

    for(i = group_position + 0; i < group_position + Cache_L2_Way_Size; i++){
        if(cache2[i].valid == 1 && cache2[i].tag==tag){
            //HIT Cache2
#ifndef TEST
    test_time += 10;
#endif

            return i;
        }
    }

    //Fail to hit cache2,replace with random algorithm

#ifndef TEST
    test_time += 200;
#endif

    srand((unsigned int)time(NULL));
    i = (rand() % Cache_L2_Way_Size) + group_position;

    /*replace by reading memory*/
    /*write back*/
    if(cache2[i].valid == 1 && cache2[i].dirty == 1){
        uint8_t ret[BURST_LEN << 1];
        uint32_t block_st = (cache2[i].tag << (Cache_L2_Group_Bit + Cache_L2_Block_Bit)) | (group_id << Cache_L2_Block_Bit);
        int w;
        memset(ret,1,sizeof ret);
        for (w = 0; w < Cache_L2_Block_Size / BURST_LEN; w++){
            public_ddr3_write(block_st + BURST_LEN * w, cache2[i].data + BURST_LEN * w,ret);
        }
    }

    /*read from memory*/
    int k;
    for(k = 0; k < (Cache_L2_Block_Size / BURST_LEN); k++){
        public_ddr3_read(block_start + (k * BURST_LEN), cache2[i].data+ (k * BURST_LEN));
    }

    cache2[i].dirty = 0;
    cache2[i].valid = 1;
    cache2[i].tag = tag;

    return i;
}

void write_cache1(hwaddr_t addr, size_t len, uint32_t data){
    uint32_t group_idx = (addr >> Cache_L1_Block_Bit) & (Cache_L1_Group_Size - 1);
    uint32_t tag = (addr >> (Cache_L1_Group_Bit + Cache_L1_Block_Bit));
    uint32_t offset = addr & (Cache_L1_Block_Size - 1);

    int i,group = group_idx * Cache_L1_Way_Size;
    for (i = group + 0;i < group + Cache_L1_Way_Size;i ++){
        if (cache1[i].valid == 1 && cache1[i].tag == tag){// WRITE HIT
            /*write through*/
            if (offset + len > Cache_L1_Block_Size){
                dram_write(addr,Cache_L1_Block_Size - offset,data);
                memcpy(cache1[i].data + offset, &data, Cache_L1_Block_Size - offset);
                /*Update Cache2*/
                write_cache2(addr,Cache_L1_Block_Size - offset,data);

                write_cache1(addr + Cache_L1_Block_Size - offset,len - (Cache_L1_Block_Size - offset),data >> (Cache_L1_Block_Size - offset));
            }else {
                dram_write(addr,len,data);
                memcpy(cache1[i].data + offset, &data, len);
                /*Update Cache2*/
                write_cache2(addr,len,data);
            }
            return;
        }
    }
    // /*not write allocate*/
    // PA3 task1
    // dram_write(addr,len,data);

    // PA3 optional task1
    write_cache2(addr,len,data);
}

void write_cache2(hwaddr_t addr, size_t len, uint32_t data){
    uint32_t group_idx = (addr >> Cache_L2_Block_Bit) & (Cache_L2_Group_Size - 1);
    uint32_t tag = (addr >> (Cache_L2_Group_Bit + Cache_L2_Block_Bit));
    uint32_t offset = addr & (Cache_L2_Block_Size - 1);

    int i,group = group_idx * Cache_L2_Way_Size;
    for (i = group + 0;i < group + Cache_L2_Way_Size;i ++){
        if (cache2[i].valid == 1 && cache2[i].tag == tag){// WRITE HIT
            cache2[i].dirty = 1;
            if (offset + len > Cache_L2_Block_Size){
                memcpy(cache2[i].data + offset, &data, Cache_L2_Block_Size - offset);
                write_cache2(addr + Cache_L2_Block_Size - offset,len - (Cache_L2_Block_Size - offset),data >> (Cache_L2_Block_Size - offset));
            }else {
                memcpy(cache2[i].data + offset, &data, len);
            }
            return;
        }
    }
     /*write allocate*/
    i = read_cache2(addr);
    cache2[i].dirty = 1;
    memcpy(cache2[i].data + offset,&data,len);
}





