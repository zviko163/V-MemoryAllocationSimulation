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
bool loadFromFile(const string &filename, vector<Job> &jobs, Memory &memory, vector<AddressRequest> &requests) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string line;
    bool memorySet = false;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // skip comments

        stringstream ss(line);
        string keyword;
        ss >> keyword;

        if (keyword == "MemorySize") {
            ss >> memory.totalSize >> memory.pageSize;
            memorySet = true;
        } else if (keyword == "Address") {
            AddressRequest req;
            ss >> req.jobName >> req.pageNumber >> req.offset;
            requests.push_back(req);
        } else {
            Job job;
            job.name = keyword;
            ss >> job.size;
            jobs.push_back(job);
        }
    }

    if (!memorySet) return false;

    memory.numFrames = memory.totalSize / memory.pageSize;
    for (int i = 0; i < memory.numFrames; ++i) {
        Frame f = {i, true, "", -1};
        memory.frames.push_back(f);
    }

    return true;
}

void resolveAddresses(const vector<AddressRequest> &requests, const vector<Job> &jobs, const Memory &memory) {
    cout << "\nAddress Resolution Results:\n";
    for (vector<AddressRequest>::const_iterator req = requests.begin(); req != requests.end(); ++req) {
        vector<Job>::const_iterator it = jobs.begin();
        for (; it != jobs.end(); ++it) {
            if (it->name == req->jobName) break;
        }
        if (it == jobs.end()) {
            cout << "Job " << req->jobName << " not found.\n";
            continue;
        }

        const Job &job = *it;
        vector<Page>::const_iterator pageIt = job.pages.begin();
        for (; pageIt != job.pages.end(); ++pageIt) {
            if (pageIt->pageNumber == req->pageNumber) break;
        }

        if (pageIt == job.pages.end()) {
            cout << "Page " << req->pageNumber << " not allocated for " << req->jobName << endl;
            continue;
        }

        int frameNumber = pageIt->frameNumber;
        int physicalAddress = (frameNumber * memory.pageSize) + req->offset;

        cout << "Job: " << req->jobName
             << ", Page: " << req->pageNumber
             << ", Offset: " << req->offset
             << " → Physical Address: " << physicalAddress << " KB (Frame " << frameNumber << ")\n";
    }
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
    for (vector<int>::iterator it = frameIndices.begin(); it != frameIndices.end(); ++it) {
        int idx = *it;
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

        for (vector<Page>::iterator p = job.pages.begin(); p != job.pages.end(); ++p)
            mainMemory.frames[p->frameNumber].isFree = true;

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
    for (vector<Page>::const_iterator p = job.pages.begin(); p != job.pages.end(); ++p)
        cout << p->pageNumber << "\t" << p->frameNumber << endl;
}

void displayMMT(const Memory &memory) {
    cout << "\nMemory Map Table (MMT):\n";
    cout << "Frame\tStatus\t\tJob(Page)\n";
    for (vector<Frame>::const_iterator f = memory.frames.begin(); f != memory.frames.end(); ++f) {
        cout << f->frameNumber << "\t";
        if (f->isFree)
            cout << "Free\t\t-\n";
        else
            cout << "Used\t\t" << f->jobName << "(" << f->pageNumber << ")\n";
    }
}
int main() {
    Memory mainMemory;
    vector<Job> jobs;
    vector<AddressRequest> requests;

    string filename;
    cout << "Enter input filename: ";
    cin >> filename;

    if (!loadFromFile(filename, jobs, mainMemory, requests)) {
        cerr << "Failed to load data.\n";
        return 1;
    }

    for (vector<Job>::iterator job = jobs.begin(); job != jobs.end(); ++job)
        divideMemoryToFrames(*job, mainMemory);

    displayMMT(mainMemory);
    for (vector<Job>::iterator job = jobs.begin(); job != jobs.end(); ++job)
        displayPMT(*job);

    if (!requests.empty())
        resolveAddresses(requests, jobs, mainMemory);

    return 0;
}
