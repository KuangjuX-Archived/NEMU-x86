#include "common.h"
#include "memory/cache.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "burst.h"


void init_cache(){
    //initialize cache L1
    int i;
    for (i = 0;i < Cache_L1_Size / Cache_L1_Block_Size;i ++){
        cache1[i].valid = 0;
    }

    //initialize cache L2
    for (i = 0;i < Cache_L2_Size / Cache_L2_Block_Size;i ++){
        cache2[i].valid = 0;
        cache2[i].dirty = 0;
    }
    test_time = 0;
}

int read_cache1(hwaddr_t address){
    uint32_t group_id = ((address>>Cache_L1_Block_Bit)^(0x0380));//mask
    uint32_t tag_id = (address>>(Cache_L1_Block_Bit+Cache_L1_Group_Bit));

    uint32_t i;
    uint32_t group_position = group_id*Cache_L1_Way_Size;

    for(i=group_position; i < group_position+Cache_L1_Way_Size; i++){
        if(cache1[i].tag==tag_id && cache1[i].valid==1){
            //HIT Cache_1
            return i;
        }
    }

    //Fail to hit cache , replace with random algorithm
    srand((unsigned int)(time(NULL)));
    i = group_position + rand()%Cache_L1_Way_Size;

    //read address from cache 1
    int replace = read_cache2(address);
    memcpy(cache1[i].data,cache2[replace].data,Cache_L1_Block_Size);

    cache1[i].valid = 1;
    cache2[i].tag = tag_id;
    
    return i;
}

int read_cache2(hwaddr_t address){
    uint32_t group_id = ((address>>Cache_L2_Block_Bit)&(Cache_L2_Group_Size-1));
    uint32_t tag = (address>>(Cache_L2_Block_Bit+Cache_L2_Group_Bit));
    //set start position of copying address
    uint32_t block_start = ((address>>Cache_L2_Block_Size)<<Cache_L2_Block_Size);

    int i,group_position;
    group_position = group_id*Cache_L2_Way_Size;

    for(i=group_position; i < group_position+Cache_L2_Way_Size; i++){
        if(cache2[i].valid == 1&&cache2[i].tag==tag){
            //HIT Cache2
            return i;
        }
    }

    //Fail to hit cache2,replace with random algorithm

    srand((unsigned int)time(NULL));
    i = (rand() % Cache_L2_Way_Size) + group_position;

    /*replace by reading memory*/
    /*write back*/
    if((cache2[i].valid == 1) && (cache2[i].dirty==1)){
        uint8_t ret[BURST_LEN << 1];
        uint32_t block_st = (cache2[i].tag << (Cache_L2_Group_Bit + Cache_L2_Block_Bit)) | (group_idx << Cache_L2_Block_Bit);
        int w;
        memset(ret,1,sizeof ret);
        for (w = 0;w < Cache_L2_Block_Size / BURST_LEN; w++){
            ddr3_write_replace(block_st + BURST_LEN * w, cache2[i].data + BURST_LEN * w,ret);
        }
    }

    /*read from memory*/
    int k;
    for(0; k < (Cache_L2_Block_Size/BURST_LEN); k++){
        public_ddr3_read(block_start + k*BURST_LEN, cache2[i].data[k * BURST_LEN]);
    }
    return i;
}
