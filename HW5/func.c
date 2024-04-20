// func.c
#include "header.h"
#include <stdio.h>

void initCache(Cache *cache) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache->blocks[i].valid = 0;
        cache->blocks[i].tag = -1;
        cache->blocks[i].lru_counter = 0;
    }
}

int accessCacheDirectMapped(Cache *cache, int address) {
    int blockNum = (address / BLOCK_SIZE) % CACHE_SIZE;
    int tag = address / (CACHE_SIZE * BLOCK_SIZE);
    int miss = 1;

    if (cache->blocks[blockNum].valid && cache->blocks[blockNum].tag == tag) {
        miss = 0;
    } else {
        cache->blocks[blockNum].valid = 1;
        cache->blocks[blockNum].tag = tag;
    }
    return miss;
}

void initAssociativeCache(AssociativeCache *cache) {
    for (int i = 0; i < NUM_SETS; i++) {
        for (int j = 0; j < 2; j++) {
            cache->sets[i].blocks[j].valid = 0;
            cache->sets[i].blocks[j].tag = -1;
            cache->sets[i].blocks[j].lru_counter = 0;
        }
    }
}

int accessCacheTwoWaySetAssociative(AssociativeCache *cache, int address) {
    int setNum = (address / BLOCK_SIZE) % NUM_SETS;
    int tag = address / (NUM_SETS * BLOCK_SIZE);
    CacheSet *set = &cache->sets[setNum];
    int miss = 1;
    int lruIndex = 0;

    for (int i = 0; i < 2; i++) {
        if (set->blocks[i].valid && set->blocks[i].tag == tag) {
            miss = 0;
            set->blocks[i].lru_counter = 0;
        } else {
            set->blocks[i].lru_counter++;
        }

        if (set->blocks[lruIndex].lru_counter < set->blocks[i].lru_counter) {
            lruIndex = i;
        }
    }

    if (miss) {
        set->blocks[lruIndex].valid = 1;
        set->blocks[lruIndex].tag = tag;
        set->blocks[lruIndex].lru_counter = 0;
    }

    return miss;
}