#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <map>
using namespace std;

struct Page {
    int pageNumber;
    int frameNumber;
};

struct Job {
    string name;
    int size;
    int pageSize;
    int numPages;
    vector<Page> pages;
};

class Memory {
    int totalFrames;
    int pageSize;
    vector<int> frames;
    map<int, string> frameToJob;

public:
    Memory(int totalFrames, int pageSize) : totalFrames(totalFrames), pageSize(pageSize) {
        frames.assign(totalFrames, -1);
    }

    // Load job pages into memory frames randomly
    void loadJob(Job &job) {
        srand(time(0));
        for (auto &page : job.pages) {
            int frame = rand() % totalFrames;
            while (frames[frame] != -1) frame = rand() % totalFrames; // find free frame
            frames[frame] = page.pageNumber;
            page.frameNumber = frame;
            frameToJob[frame] = job.name;
        }
    }

    // Display memory status
    void showMemory() {
        cout << "\n=== MEMORY FRAMES ===\n";
        for (int i = 0; i < totalFrames; i++) {
            if (frames[i] == -1)
                cout << "Frame " << i << ": [Empty]\n";
            else
                cout << "Frame " << i << ": Page " << frames[i] << " (" << frameToJob[i] << ")\n";
        }
    }

    // Perform Address Resolution
    void addressResolution(Job &job, int logicalAddress) {
        int pageNumber = logicalAddress / job.pageSize;
        int offset = logicalAddress % job.pageSize;

        if (pageNumber >= job.numPages) {
            cout << "Invalid logical address! (Exceeds job size)\n";
            return;
        }

        int frameNumber = job.pages[pageNumber].frameNumber;
        int physicalAddress = frameNumber * job.pageSize + offset;

        cout << "\nAddress Resolution for Job: " << job.name << endl;
        cout << "Logical Address: " << logicalAddress << endl;
        cout << "→ Page Number: " << pageNumber << ", Offset: " << offset << endl;
        cout << "→ Frame Number: " << frameNumber << endl;
        cout << "→ Physical Address: " << physicalAddress << endl;
    }
};

int main() {
    int totalFrames, pageSize, numJobs;
    cout << "Enter total number of memory frames: ";
    cin >> totalFrames;
    cout << "Enter page size (in bytes): ";
    cin >> pageSize;

    Memory memory(totalFrames, pageSize);

    cout << "Enter number of jobs: ";
    cin >> numJobs;

    vector<Job> jobs(numJobs);

    // Accept multiple jobs
    for (int i = 0; i < numJobs; i++) {
        cout << "\nEnter name of Job " << i + 1 << ": ";
        cin >> jobs[i].name;
        cout << "Enter size of Job " << jobs[i].name << " (in bytes): ";
        cin >> jobs[i].size;
        jobs[i].pageSize = pageSize;
        jobs[i].numPages = (jobs[i].size + pageSize - 1) / pageSize;

        cout << "Job divided into " << jobs[i].numPages << " pages.\n";

        for (int p = 0; p < jobs[i].numPages; p++) {
            Page page;
            page.pageNumber = p;
            page.frameNumber = -1;
            jobs[i].pages.push_back(page);
        }

        // Load pages into memory randomly
        memory.loadJob(jobs[i]);
    }

    memory.showMemory();

    // Perform Address Resolution
    string jobName;
    cout << "\nEnter job name for address resolution: ";
    cin >> jobName;

    bool found = false;
    for (auto &job : jobs) {
        if (job.name == jobName) {
            int logicalAddress;
            cout << "Enter logical address: ";
            cin >> logicalAddress;
            memory.addressResolution(job, logicalAddress);
            found = true;
            break;
        }
    }

    if (!found)
        cout << "Job not found!\n";

    return 0;
}
