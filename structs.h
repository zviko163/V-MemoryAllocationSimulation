#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <vector>
using namespace std;

// Represents a single page belonging to a job
struct Page {
    int pageNumber;
    int frameNumber; // -1 if not allocated
};

// Represents a job
struct Job {
    string name;
    int size;          // job size in KB
    int numPages;      // computed from job size and page size
    vector<Page> pages;
};

// Represents a single frame in main memory
struct Frame {
    int frameNumber;
    bool isFree;
    string jobName;
    int pageNumber;
};

// Represents the entire main memory
struct Memory {
    int totalSize;
    int pageSize;
    int numFrames;
    vector<Frame> frames;
};

#endif
