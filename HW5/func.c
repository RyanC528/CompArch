// func.c
#include "header.h"
#include <stdio.h>

// Initialize all blocks in a direct-mapped cache to invalid
void initCache(Cache *cache) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache->blocks[i].valid = 0;
        cache->blocks[i].tag = -1;
        cache->blocks[i].lru_counter = 0;
    }
}

// Access the direct-mapped cache with a given memory address
int accessCacheDirectMapped(Cache *cache, int address) {
    int blockNum = (address / BLOCK_SIZE) % CACHE_SIZE;  // Calculate block index within the cache
    int tag = address / (CACHE_SIZE * BLOCK_SIZE);  // Calculate tag from address
    int miss = 1;  // Assume a miss unless a hit is detected

    if (cache->blocks[blockNum].valid && cache->blocks[blockNum].tag == tag) {
        miss = 0;  // Hit
    } else {
        // Update the block information on a miss
        cache->blocks[blockNum].valid = 1;
        cache->blocks[blockNum].tag = tag;
    }
    return miss;  // Return whether it was a miss or not
}

// Initialize a two-way set associative cache
void initAssociativeCache(AssociativeCache *cache) {
    for (int i = 0; i < NUM_SETS; i++) {
        for (int j = 0; j < 2; j++) {
            cache->sets[i].blocks[j].valid = 0;
            cache->sets[i].blocks[j].tag = -1;
            cache->sets[i].blocks[j].lru_counter = 0;
        }
    }
}

// Access the two-way set associative cache with a given memory address
int accessCacheTwoWaySetAssociative(AssociativeCache *cache, int address) {
    int setNum = (address / BLOCK_SIZE) % NUM_SETS;  // Calculate which set to use
    int tag = address / (NUM_SETS * BLOCK_SIZE);  // Calculate tag from address
    CacheSet *set = &cache->sets[setNum];
    int miss = 1;  // Assume a miss unless a hit is detected
    int lruIndex = 0;  // Track least recently used block index

    // Check for hit and update LRU counters
    for (int i = 0; i < 2; i++) {
        if (set->blocks[i].valid && set->blocks[i].tag == tag) {
            miss = 0;
            set->blocks[i].lru_counter = 0;  // Reset counter on hit
        } else {
            set->blocks[i].lru_counter++;  // Increment LRU counter
        }

        // Identify the least recently used block
        if (set->blocks[lruIndex].lru_counter < set->blocks[i].lru_counter) {
            lruIndex = i;
        }
    }

    // Replace the LRU block on a miss
    if (miss) {
        set->blocks[lruIndex].valid = 1;
        set->blocks[lruIndex].tag = tag;
        set->blocks[lruIndex].lru_counter = 0;  // Reset counter for the new block
    }

    return miss;  // Return whether it was a miss
}