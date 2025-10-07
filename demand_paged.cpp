#include "structs.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <ctime>
#include <random>
#include <algorithm>
using namespace std;


// function to load data from a file
/*
    This function loads data from a file and stores it in the jobs, memory, and requests vectors.
*/

bool loadFromFile(const string &filename, vector<Job> &jobs, Memory &memory, vector<AddressRequest> &requests) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string line;
    bool memorySet = false;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
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

// function to simulate demand paging
/*
    This function simulates demand paging for a given job.
*/

void simulateDemandPaging(Job &job, Memory &mainMemory) {
    job.numPages = ceil((double)job.size / mainMemory.pageSize);
    cout << "\nLoading Job " << job.name << " (" << job.size << " KB)...\n";
    cout << "Total Pages: " << job.numPages << endl;

    // Initialize all pages as "not in memory"
    job.pages.clear();
    for (int p = 0; p < job.numPages; ++p) {
        Page page = {p, -1};
        job.pages.push_back(page);
    }

    // Randomize frame indices
    vector<int> frameIndices(mainMemory.numFrames);
    for (int i = 0; i < mainMemory.numFrames; ++i) frameIndices[i] = i;
    unsigned seed = time(nullptr);
    shuffle(frameIndices.begin(), frameIndices.end(), default_random_engine(seed));

    int framesToLoad = min(job.numPages, (int)(job.numPages * 0.5 + rand() % (job.numPages / 2 + 1))); 
    // Load 50–100% of pages randomly

    cout << "Pages loaded into memory: " << framesToLoad << endl;

    int allocated = 0;
    for (int i = 0; i < mainMemory.numFrames && allocated < framesToLoad; ++i) {
        int frameIdx = frameIndices[i];
        if (!mainMemory.frames[frameIdx].isFree) continue;

        int pageToLoad = rand() % job.numPages;
        if (job.pages[pageToLoad].frameNumber != -1) continue; // already loaded

        mainMemory.frames[frameIdx].isFree = false;
        mainMemory.frames[frameIdx].jobName = job.name;
        mainMemory.frames[frameIdx].pageNumber = pageToLoad;

        job.pages[pageToLoad].frameNumber = frameIdx;
        allocated++;
    }
}

// function to display the page map table
/*
    This function displays the page map table for a given job.
*/

void displayPMT(const Job &job) {
    cout << "\nPage Map Table (PMT) for " << job.name << ":\n";
    cout << "Page\tFrame\n";
    for (vector<Page>::const_iterator p = job.pages.begin(); p != job.pages.end(); ++p) {
        cout << p->pageNumber << "\t";
        if (p->frameNumber == -1) cout << "Not in memory\n";
        else cout << p->frameNumber << endl;
    }
}

// function to display the memory map table
/*
    This function displays the memory map table.
*/

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

// function to resolve addresses
/*
    This function resolves logical addresses to physical addresses using the page table and memory map table.
*/

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
        if (req->pageNumber >= job.numPages) {
            cout << "Invalid page number for job " << job.name << endl;
            continue;
        }

        const Page &page = job.pages[req->pageNumber];
        if (page.frameNumber == -1) {
            cout << "Page Fault! " << job.name << " Page " << req->pageNumber << " not in memory.\n";
        } else {
            int physicalAddress = (page.frameNumber * memory.pageSize) + req->offset;
            cout << "Job: " << req->jobName
                 << " | Page: " << req->pageNumber
                 << " | Offset: " << req->offset
                 << " → Physical Address: " << physicalAddress << " KB (Frame " << page.frameNumber << ")\n";
        }
    }
}

// main function
/*
    This function is the main function that runs the program.
*/

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

    srand(time(0));
    for (vector<Job>::iterator job = jobs.begin(); job != jobs.end(); ++job)
        simulateDemandPaging(*job, mainMemory);

    displayMMT(mainMemory);
    for (vector<Job>::const_iterator job = jobs.begin(); job != jobs.end(); ++job)
        displayPMT(*job);

    if (!requests.empty())
        resolveAddresses(requests, jobs, mainMemory);

    return 0;
}
