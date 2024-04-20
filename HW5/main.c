// main.c
#include "header.h"
#include <stdio.h>
#include <stdlib.h>

void simulateCacheAccessFromFile(const char *filename, Cache *directCache, AssociativeCache *assocCache) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char type;
    int address;
    int directMissCount = 0, assocMissCount = 0;

    while (fscanf(file, "%c: %d\n", &type, &address) == 2) {
        directMissCount += accessCacheDirectMapped(directCache, address);
        assocMissCount += accessCacheTwoWaySetAssociative(assocCache, address);
    }

    printf("Results for %s:\nDirect Mapped Misses: %d\nTwo-Way Set Associative Misses: %d\n", filename, directMissCount, assocMissCount);
    fclose(file);
}

int main() {
    Cache directCache;
    AssociativeCache assocCache;

    initCache(&directCache);
    initAssociativeCache(&assocCache);

    printf("Simulating Naive Matrix Multiplication\n");
    simulateCacheAccessFromFile("naive4-trace.txt", &directCache, &assocCache);

    initCache(&directCache);  // Re-initialize the cache for the next simulation
    initAssociativeCache(&assocCache);
    
    printf("Simulating Smart Matrix Multiplication\n");
    simulateCacheAccessFromFile("smart4-trace.txt", &directCache, &assocCache);

    return 0;
}