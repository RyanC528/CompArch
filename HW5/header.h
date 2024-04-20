// header.h
#ifndef HEADER_H
#define HEADER_H

#define N 4
#define CACHE_SIZE 8  // 8 blocks in the cache
#define BLOCK_SIZE 2  // Each block holds 2 words
#define MEM_SIZE (4*4*3)  // Total size of A, B, and C (each is 4x4)
#define NUM_SETS (CACHE_SIZE / 2)

// Define cache block structure
typedef struct {
    int valid;
    int tag;
    int lru_counter;
} CacheBlock;

// Define direct-mapped cache structure
typedef struct {
    CacheBlock blocks[CACHE_SIZE];
} Cache;

// Define two-way set associative cache structure
typedef struct {
    CacheBlock blocks[2];  // Two blocks per set
} CacheSet;

typedef struct {
    CacheSet sets[NUM_SETS];
} AssociativeCache;

// Function prototypes
void initCache(Cache *cache);
void initAssociativeCache(AssociativeCache *cache);
int accessCacheDirectMapped(Cache *cache, int address);
int accessCacheTwoWaySetAssociative(AssociativeCache *cache, int address);
void simulateCacheAccessFromFile(const char *filename, Cache *directCache, AssociativeCache *assocCache);

#endif // HEADER_H