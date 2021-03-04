// based on cs3650 starter code

#include "bitmap.h"
#include <stdio.h>

int
bitmap_get(void* bm, int ii)
{
    void* loc = (void*)(bm + (sizeof(int) * ii));
    return *(int*)(loc);
    
}

void
bitmap_put(void* bm, int ii, int vv)
{

    void* loc = (void*)(bm + (sizeof(int) * ii));
    int* num = loc;
    *num = ii;

}

void
bitmap_print(void* bm, int size)
{
    int* nums = bm;
    for(int i = 0; i < size; i += sizeof(int)) {
	    printf("i = %d, num = %d", i, nums[i]);
    }
    return;

}
