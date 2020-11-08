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

    for(i=group_position; i<Cache_L1_Way_Size; i++){
        if(cache1[i].tag==tag_id && cache1[i].valid==1){
            //HIT Cache_1
            return i;
        }
    }

    //If fail to hit cache , replace with random algorithm
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
    return -1;
}