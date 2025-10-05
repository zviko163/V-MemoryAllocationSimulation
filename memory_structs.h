#ifndef MEMORY_STRUCTS_H
#define MEMORY_STRUCTS_H

#include <string>
#include <vector>
using namespace std;

// ---------- STRUCT DEFINITIONS ----------

// Each page entry (page number + its frame number)
struct PageMapEntry {
    int pageNumber;
    int frameNumber;
};

// Each job has its own PMT
struct PageMapTable {
    vector<PageMapEntry> entries;
};

// Each frame in main memory
struct MemoryMapEntry {
    int frameNumber;
    string jobName;
    int pageNumber;
};

// The full memory map (represents main memory)
struct MemoryMapTable {
    vector<MemoryMapEntry> entries;
};

// Each job (program)
struct Job {
    string name;
    int jobSize;
    int pageSize;
    int numPages;
    int internalFragmentation;
    PageMapTable pmt; // PMT belongs to each job
};

#endif
