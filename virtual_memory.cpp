#include "structs.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <ctime>
#include <random>
#include <algorithm>
#include <queue>
#include <list>
using namespace std;

bool loadFromFile(const string &filename, vector<Job> &jobs, Memory &memory,
                  vector<AddressRequest> &requests, int &replacementPolicy) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string line;
    bool memorySet = false;
    bool policySet = false;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        stringstream ss(line);
        string keyword;
        ss >> keyword;

        if (keyword == "MemorySize") {
            ss >> memory.totalSize >> memory.pageSize;
            memorySet = true;
        } else if (keyword == "ReplacementPolicy") {
            ss >> replacementPolicy;
            policySet = true;
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

    if (!memorySet || !policySet) return false;

    memory.numFrames = memory.totalSize / memory.pageSize;
    for (int i = 0; i < memory.numFrames; ++i) {
        Frame f = {i, true, "", -1};
        memory.frames.push_back(f);
    }

    return true;
}

// Simulate initial page load
void simulateDemandPaging(Job &job, Memory &mainMemory) {
    job.numPages = ceil((double)job.size / mainMemory.pageSize);
    cout << "\nLoading Job " << job.name << " (" << job.size << " KB)...\n";
    cout << "Total Pages: " << job.numPages << endl;

    job.pages.clear();
    for (int p = 0; p < job.numPages; ++p) {
        Page page = {p, -1};
        job.pages.push_back(page);
    }

    // Randomly preload some pages
    vector<int> frameIndices(mainMemory.numFrames);
    for (int i = 0; i < mainMemory.numFrames; ++i) frameIndices[i] = i;
    shuffle(frameIndices.begin(), frameIndices.end(), default_random_engine(time(nullptr)));

    int framesToLoad = min(job.numPages, (int)(job.numPages * 0.5 + rand() % (job.numPages / 2 + 1)));
    int allocated = 0;
    for (int i = 0; i < mainMemory.numFrames && allocated < framesToLoad; ++i) {
        int frameIdx = frameIndices[i];
        if (!mainMemory.frames[frameIdx].isFree) continue;

        int pageToLoad = rand() % job.numPages;
        if (job.pages[pageToLoad].frameNumber != -1) continue;

        mainMemory.frames[frameIdx].isFree = false;
        mainMemory.frames[frameIdx].jobName = job.name;
        mainMemory.frames[frameIdx].pageNumber = pageToLoad;
        job.pages[pageToLoad].frameNumber = frameIdx;
        allocated++;
    }
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

void displayPMT(const Job &job) {
    cout << "\nPage Map Table (PMT) for " << job.name << ":\n";
    cout << "Page\tFrame\n";
    for (vector<Page>::const_iterator p = job.pages.begin(); p != job.pages.end(); ++p) {
        cout << p->pageNumber << "\t";
        if (p->frameNumber == -1) cout << "Not in memory\n";
        else cout << p->frameNumber << endl;
    }
}

void resolveAddresses(vector<AddressRequest> &requests, vector<Job> &jobs, Memory &memory, int replacementPolicy) {
    cout << "\nAddress Resolution with Page Replacement (" 
         << (replacementPolicy == 0 ? "FIFO" : "LRU") << "):\n";

    queue<int> fifoQueue;
    list<int> lruList;

    for (vector<AddressRequest>::iterator req = requests.begin(); req != requests.end(); ++req) {
        vector<Job>::iterator jobIt = jobs.begin();
        for (; jobIt != jobs.end(); ++jobIt) {
            if (jobIt->name == req->jobName) break;
        }
        if (jobIt == jobs.end()) {
            cout << "Job " << req->jobName << " not found.\n";
            continue;
        }
        Job &job = *jobIt;
        if (req->pageNumber >= job.numPages) {
            cout << "Invalid page number for " << job.name << endl;
            continue;
        }

        Page &page = job.pages[req->pageNumber];
        int frameNum = page.frameNumber;

        if (frameNum == -1) {
            cout << "\nPage Fault! " << job.name << " Page " << req->pageNumber << " not in memory.\n";

            // Try to find a free frame
            int freeFrame = -1;
            for (vector<Frame>::iterator f = memory.frames.begin(); f != memory.frames.end(); ++f) {
                if (f->isFree) { freeFrame = f->frameNumber; break; }
            }

            // If no free frame, replace based on policy
            if (freeFrame == -1) {
                int victimFrame;
                if (replacementPolicy == 0) {
                    victimFrame = fifoQueue.front(); fifoQueue.pop();
                    cout << "Replacing Frame " << victimFrame << " (FIFO)\n";
                } else {
                    victimFrame = lruList.back(); lruList.pop_back();
                    cout << "Replacing Frame " << victimFrame << " (LRU)\n";
                }

                // Invalidate victim
                for (vector<Job>::iterator j = jobs.begin(); j != jobs.end(); ++j) {
                    for (vector<Page>::iterator pg = j->pages.begin(); pg != j->pages.end(); ++pg) {
                        if (pg->frameNumber == victimFrame) pg->frameNumber = -1;
                    }
                }

                freeFrame = victimFrame;
            }

            // Load new page
            memory.frames[freeFrame].isFree = false;
            memory.frames[freeFrame].jobName = job.name;
            memory.frames[freeFrame].pageNumber = req->pageNumber;
            job.pages[req->pageNumber].frameNumber = freeFrame;

            if (replacementPolicy == 0) fifoQueue.push(freeFrame);
            else {
                lruList.remove(freeFrame);
                lruList.push_front(freeFrame);
            }

            frameNum = freeFrame;
        } else {
            // Page hit
            cout << "Page Hit: " << job.name << " Page " << req->pageNumber << endl;
            if (replacementPolicy == 1) {
                lruList.remove(frameNum);
                lruList.push_front(frameNum);
            }
        }

        int physicalAddress = frameNum * memory.pageSize + req->offset;
        cout << "Job: " << req->jobName << " | Page: " << req->pageNumber
             << " | Offset: " << req->offset
             << " → Physical Address: " << physicalAddress << " KB (Frame " << frameNum << ")\n";

        displayMMT(memory);
    }
}

int main() {
    Memory mainMemory;
    vector<Job> jobs;
    vector<AddressRequest> requests;
    int replacementPolicy = 0;

    string filename;
    cout << "Enter input filename: ";
    cin >> filename;

    if (!loadFromFile(filename, jobs, mainMemory, requests, replacementPolicy)) {
        cerr << "Failed to load data. Ensure MemorySize and ReplacementPolicy exist.\n";
        return 1;
    }

    srand(time(0));
    for (vector<Job>::iterator job = jobs.begin(); job != jobs.end(); ++job)
        simulateDemandPaging(*job, mainMemory);

    displayMMT(mainMemory);
    for (vector<Job>::iterator job = jobs.begin(); job != jobs.end(); ++job)
        displayPMT(*job);

    if (!requests.empty())
        resolveAddresses(requests, jobs, mainMemory, replacementPolicy);

    return 0;
}
