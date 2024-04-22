// main.c
#include "header.h"
#include <stdio.h>
#include <stdlib.h>



// Simulate cache accesses from a trace file
void simulateCacheAccessFromFile(const char *filename, Cache *directCache, AssociativeCache *assocCache) {
    FILE *file = fopen(filename, "r");  // Open the trace file
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char type;  // Type of access ('R' for read, 'W' for write)
    int address;  // Memory address being accessed
    int directMissCount = 0, assocMissCount = 0, totalNum = 0;  // Counters for misses

    // Process each trace entry
    while (fscanf(file, "%c: %d\n", &type, &address) == 2) {
        totalNum ++;
        directMissCount += accessCacheDirectMapped(directCache, address);
        assocMissCount += accessCacheTwoWaySetAssociative(assocCache, address);
    }

    // Print results for the file
    printf("Results for %s:\nDirect Mapped Misses: %d\nTwo-Way Set Associative Misses: %d\nTotal Number of Access Attemps: %d\n", filename, directMissCount, assocMissCount,totalNum);
    fclose(file);  // Close the file
}

// Main function to run the cache simulation for both trace files
int main() {
    Cache directCache;  // Direct-mapped cache
    AssociativeCache assocCache;  // Two-way set associative cache
    

    // Initialize both caches
    initCache(&directCache);
    initAssociativeCache(&assocCache);

    // Simulate the naive matrix multiplication trace
    printf("Simulating Naive Matrix Multiplication\n");
    simulateCacheAccessFromFile("naive4-trace.txt", &directCache, &assocCache);

    // Re-initialize the caches for the next simulation
    initCache(&directCache);
    initAssociativeCache(&assocCache);

    // Simulate the smart matrix multiplication trace
    printf("Simulating Smart Matrix Multiplication\n");
    simulateCacheAccessFromFile("smart4-trace.txt", &directCache, &assocCache);

    printCache(&directCache);

    return 0;
}