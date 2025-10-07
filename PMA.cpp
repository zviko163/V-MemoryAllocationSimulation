#include "structs.h"
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>  // for std::shuffle
#include <random>     // for std::default_random_engine
#include <ctime>      // for std::time

using namespace std;

/*
 * function to load jobs and memory info from a text file
 * this makes it easier to test with different inputs
 * file format:
 * MemorySize <total_memory_size_in_KB> <page_size_in_KB>
 * Job1 <job_size_in_KB>
 * Job2 <job_size_in_KB>
 * ...
 */
bool loadFromFile(const string &filename, vector<Job> &jobs, Memory &memory) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }

    string line;
    bool memorySet = false;

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string name;
        ss >> name;

        // extraacting memory attributes and job details from file
        if (name == "MemorySize") {
            ss >> memory.totalSize >> memory.pageSize;
            memorySet = true;
        } else {
            Job job;
            job.name = name;
            ss >> job.size;
            jobs.push_back(job);
        }
    }

    file.close();

    // ensuring memory parameters were set
    if (!memorySet) {
        cerr << "Error: Memory size not specified in file.\n";
        return false;
    }

    // computing number of frames created in memory
    memory.numFrames = memory.totalSize / memory.pageSize;

    // initializing memory frames
    for (int i = 0; i < memory.numFrames; ++i) {
        Frame f;
        f.frameNumber = i;
        f.isFree = true;
        f.jobName = "";
        f.pageNumber = -1;
        memory.frames.push_back(f);
    }

    return true;
}


/*
 * function to calculate the internal fragmentation for a job
 * returns the fragmentation size in KB
 */
int calculateInternalFragmentation(const Job &job, int pageSize) {
    int remainder = job.size % pageSize;
    if (remainder == 0)
        return 0;
    return pageSize - remainder;
}

/*
 * divides memory into frames based on page size (specified in the input by end user)
 * allocates memory frames to the job based on its size and the main memory's page size
 * if memory is insufficient, it does not allocate any frames to the job
 */

void divideMemoryToFrames(Job &job, Memory &mainMemory) {
    // calculating number of pages per job using job size and page size
    job.numPages = ceil((double)job.size / mainMemory.pageSize);

    cout << "\nAllocating job " << job.name << " (" << job.size << " KB)"
         << " needing " << job.numPages << " pages...\n";

    int allocated = 0;

    // create a list of all frame indices
    vector<int> frameIndices(mainMemory.numFrames);
    for (int i = 0; i < mainMemory.numFrames; ++i)
        frameIndices[i] = i;

    // shuffle frame indices for random allocation
    unsigned seed = static_cast<unsigned>(time(nullptr));
    shuffle(frameIndices.begin(), frameIndices.end(), default_random_engine(seed));

    // allocate frames in the randomized order
    for (int idx : frameIndices) {
        if (allocated >= job.numPages)
            break;

        if (mainMemory.frames[idx].isFree) {
            mainMemory.frames[idx].isFree = false;
            mainMemory.frames[idx].jobName = job.name;
            mainMemory.frames[idx].pageNumber = allocated;

            // update Page Map Table
            Page page = {allocated, idx};
            job.pages.push_back(page);

            allocated++;
        }
    }

    // if not all pages could be allocated, roll back
    if (allocated < job.numPages) {
        cout << "Not enough memory to allocate all pages for " << job.name << endl;

        for (Page &p : job.pages)
            mainMemory.frames[p.frameNumber].isFree = true;

        job.pages.clear();
    } else {
        cout << "Job " << job.name << " allocated successfully.\n";

        int fragmentation = calculateInternalFragmentation(job, mainMemory.pageSize);
        if (fragmentation > 0)
            cout << "\nInternal Fragmentation for job " << job.name << ": "
                 << fragmentation << " KB\n";
        else
            cout << "No Internal Fragmentation for job " << job.name << ".\n";
    }
}

/*
 * displaying functions
 * functions to display Page Map Table (PMT) and Memory Map Table (MMT)
 */
void displayPMT(const Job &job) {
    cout << "\nPage Map Table (PMT) for " << job.name << ":\n";
    cout << "Page\tFrame\n";
    for (const Page &p : job.pages)
        cout << p.pageNumber << "\t" << p.frameNumber << endl;
}

void displayMMT(const Memory &memory) {
    cout << "\nMemory Map Table (MMT):\n";
    cout << "Frame\tStatus\t\tJob(Page)\n";
    for (const Frame &f : memory.frames) {
        cout << f.frameNumber << "\t";
        if (f.isFree)
            cout << "Free\t\t-\n";
        else
            cout << "Used\t\t" << f.jobName << "(" << f.pageNumber << ")\n";
    }
}

int main() {
    Memory mainMemory;
    vector<Job> jobs;

    string filename;
    cout << "Enter input filename: ";
    cin >> filename;

    if (!loadFromFile(filename, jobs, mainMemory)) {
        cerr << "Failed to load data.\n";
        return 1;
    }

    // Allocate memory for each job
    for (auto &job : jobs)
        divideMemoryToFrames(job, mainMemory);

    // Display MMT and PMTs
    displayMMT(mainMemory);
    for (auto &job : jobs)
        displayPMT(job);

    return 0;
}
