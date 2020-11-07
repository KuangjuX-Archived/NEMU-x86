#include "memory/cache.h"
#include "common.h"


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