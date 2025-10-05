#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>   // for rand()
#include "memory_structs.h"

using namespace std;

// ---------- GLOBAL MEMORY ----------
MemoryMapTable mmt;
vector<Job> jobTable;
int totalFrames = 8;  // example total frames in memory


// ---------- FUNCTION DECLARATIONS ----------
void acceptJob();
void divideIntoPages(Job &job);
void loadJobIntoFrames(Job &job);
void performAddressResolution(Job &job);


// ---------- MAIN FUNCTION ----------
int main() {
    cout << "=== Paged Memory Allocation Simulation ===" << endl;
    
    // Initialize memory map table
    for (int i = 0; i < totalFrames; i++) {
        MemoryMapEntry entry = {i, "EMPTY", -1};
        mmt.entries.push_back(entry);
    }

    // Accept and process one job (for now)
    acceptJob();

    // Display memory map (to visualize)
    cout << "\n=== Memory Map Table ===" << endl;
    for (auto &e : mmt.entries) {
        cout << "Frame " << e.frameNumber << " | Job: " 
             << e.jobName << " | Page: " << e.pageNumber << endl;
    }

    return 0;
}


// ---------- FUNCTION DEFINITIONS ----------

// Accept job info from user
void acceptJob() {
    Job job;
    cout << "Enter Job Name: ";
    cin >> job.name;
    cout << "Enter Job Size (KB): ";
    cin >> job.jobSize;
    cout << "Enter Page Size (KB): ";
    cin >> job.pageSize;

    divideIntoPages(job);
    loadJobIntoFrames(job);
    performAddressResolution(job);

    // adding the user-entered job to the job table
    jobTable.push_back(job);
}

// Divide job into pages
void divideIntoPages(Job &job) {
    job.numPages = ceil((double)job.jobSize / job.pageSize);
    job.internalFragmentation = (job.numPages * job.pageSize) - job.jobSize;

    cout << "\nJob divided into " << job.numPages << " pages.";
    cout << "\nInternal Fragmentation: " << job.internalFragmentation << " KB\n";

    // Initialize PMT
    for (int i = 0; i < job.numPages; i++) {
        PageMapEntry entry = {i, -1}; // frame to be assigned later
        job.pmt.entries.push_back(entry);
    }
}

// Load job pages into available frames (randomly for simulation)
void loadJobIntoFrames(Job &job) {
    cout << "\nLoading pages into memory frames...\n";

    for (int i = 0; i < job.numPages; i++) {
        int frame = rand() % totalFrames;

        // Assign frame in PMT
        job.pmt.entries[i].frameNumber = frame;

        // Update MMT
        mmt.entries[frame].jobName = job.name;
        mmt.entries[frame].pageNumber = i;

        cout << "Page " << i << " -> Frame " << frame << endl;
    }
}

// Perform logical → physical address translation
void performAddressResolution(Job &job) {
    cout << "\nAddress Resolution for Job: " << job.name << endl;
    int logicalAddr;
    cout << "Enter Logical Address (e.g., 250): ";
    cin >> logicalAddr;

    int pageNumber = logicalAddr / job.pageSize;
    int offset = logicalAddr % job.pageSize;

    if (pageNumber >= job.numPages) {
        cout << "Invalid address — exceeds job size.\n";
        return;
    }

    int frame = job.pmt.entries[pageNumber].frameNumber;
    int physicalAddr = (frame * job.pageSize) + offset;

    cout << "Logical Address " << logicalAddr 
         << " → Physical Address " << physicalAddr << endl;
}
