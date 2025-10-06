#include "structs.h"
#include <iostream>
#include <cmath>

using namespace std;

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

    // looping through frames array to allocate pages to the free frames
    for (int i = 0; i < mainMemory.numFrames && allocated < job.numPages; ++i) {
        if (mainMemory.frames[i].isFree) {
            mainMemory.frames[i].isFree = false;
            mainMemory.frames[i].jobName = job.name;
            mainMemory.frames[i].pageNumber = allocated;

            // updating job's Page Map Table; adding allocated page
            Page page = {allocated, i};
            job.pages.push_back(page);

            allocated++;
        }
    }

    // in an scenario where not all pages could be allocated, deallocate all previously allocated pages
    if (allocated < job.numPages) {
        cout << "Not enough memory to allocate all pages for " << job.name << endl;

        for (Page &p : job.pages)
            mainMemory.frames[p.frameNumber].isFree = true;

        // clearing the job's pages array
        job.pages.clear();
    } else {
        // on a successful allocation
        cout << "Job " << job.name << " allocated successfully.\n";
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

    // User defines memory size and page size
    cout << "Enter total memory size (KB): ";
    cin >> mainMemory.totalSize;

    cout << "Enter page size (KB): ";
    cin >> mainMemory.pageSize;

    mainMemory.numFrames = mainMemory.totalSize / mainMemory.pageSize;

    // Initialize memory frames
    for (int i = 0; i < mainMemory.numFrames; ++i)
        mainMemory.frames.push_back({i, true, "", -1});

    cout << "\nMemory initialized with " << mainMemory.numFrames
         << " frames (" << mainMemory.pageSize << " KB each).\n";

    int numJobs;
    cout << "\nEnter number of jobs to allocate: ";
    cin >> numJobs;

    vector<Job> jobs;
    for (int i = 0; i < numJobs; ++i) {
        Job job;
        cout << "\nEnter job name: ";
        cin >> job.name;
        cout << "Enter job size (KB): ";
        cin >> job.size;

        divideMemoryToFrames(job, mainMemory);
        jobs.push_back(job);
    }

    // Display results
    displayMMT(mainMemory);

    for (auto &job : jobs)
        displayPMT(job);

    return 0;
}
