// header.h
#ifndef HEADER_H
#define HEADER_H

// Constants for matrix dimensions and cache configuration
#define N 4  // Dimension of the matrices A, B, and C
#define CACHE_SIZE 8  // Number of blocks in the cache
#define BLOCK_SIZE 2  // Number of words in each cache block
#define MEM_SIZE (4*4*3)  // Total memory size needed for matrices A, B, and C
#define NUM_SETS (CACHE_SIZE / 2)  // Number of sets in the associative cache

// Structure to represent a single cache block
typedef struct {
    int valid;  // Valid bit to indicate if the block contains data
    int tag;  // Tag to identify the block of memory
    int lru_counter;  // Counter for implementing LRU policy
} CacheBlock;

// Structure for a direct-mapped cache
typedef struct {
    CacheBlock blocks[CACHE_SIZE];  // Array of cache blocks
} Cache;

// Structure for a set in a two-way set associative cache
typedef struct {
    CacheBlock blocks[2];  // Two blocks per set, for the two-way set associative cache
} CacheSet;

// Structure for the entire two-way set associative cache
typedef struct {
    CacheSet sets[NUM_SETS];  // Array of sets
} AssociativeCache;

// Prototypes for functions defined in func.c
void initCache(Cache *cache);
void initAssociativeCache(AssociativeCache *cache);
int accessCacheDirectMapped(Cache *cache, int address);
int accessCacheTwoWaySetAssociative(AssociativeCache *cache, int address);
void simulateCacheAccessFromFile(const char *filename, Cache *directCache, AssociativeCache *assocCache);
void printCache(Cache *cache);
void printAssociativeCache(AssociativeCache *cache);

#endif // HEADER_H