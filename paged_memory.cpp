// paged_memory.cpp
// Compile: g++ -O2 page_memory.cpp -o page_memory

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <limits>
#include <unordered_map>
#include <random>
#include <chrono>

using namespace std;

/*
Design:
- Page size in bytes provided by user.
- Physical memory represented as N frames (user-specified).
- Each job has:
    id, size (bytes), num_pages, internal_fragmentation, page table (page -> frame or -1)
- frames[] keeps (job_id, page_no) or (-1,-1) if free.
- Two modes:
    1) Paged Memory Allocation (single job) - loads pages randomly into free frames once.
    2) Demand Paged Memory Allocation (multiple jobs) - pages loaded only on access;
       if no free frames, choose a random frame to evict (random replacement).
- Address resolution:
    Input: job id and logical address (byte). Compute page_no = logical_addr / page_size,
    offset = logical_addr % page_size. If page present -> report physical address (frame*page_size + offset).
    If not present:
        - non-demand mode: report page not loaded
        - demand mode: simulate page fault, load it (possibly evict random), then resolve.
*/

struct PageRef {
    int job_id;
    int page_no;
    PageRef(): job_id(-1), page_no(-1) {}
    PageRef(int j, int p): job_id(j), page_no(p) {}
};

struct Job {
    int id;
    long long size; // bytes
    int num_pages;
    long long internal_frag; // bytes
    vector<int> page_table; // page -> frame number or -1
    Job(int id_=0, long long size_=0, int num_pages_=0, long long internal_frag_=0)
        : id(id_), size(size_), num_pages(num_pages_), internal_frag(internal_frag_) {}
};

static std::mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());

int get_int_input(const string &prompt) {
    while (true) {
        cout << prompt;
        int v;
        if (cin >> v) return v;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input, try again.\n";
    }
}

long long get_ll_input(const string &prompt) {
    while (true) {
        cout << prompt;
        long long v;
        if (cin >> v) return v;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input, try again.\n";
    }
}

void press_enter_to_continue() {
    cout << "Press Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void show_frames(const vector<PageRef> &frames, int page_size) {
    cout << "\nPhysical frames (frame_index -> job_id:page_no or FREE):\n";
    for (size_t i=0;i<frames.size();++i) {
        cout << " Frame[" << i << "] -> ";
        if (frames[i].job_id == -1) cout << "FREE\n";
        else cout << "Job " << frames[i].job_id << " : Page " << frames[i].page_no
                  << "  (phys addr range " << (i*page_size) << " - " << (i*page_size + page_size - 1) << ")\n";
    }
    cout << endl;
}

void mode_paged_single_job() {
    cout << "\n=== Paged Memory Allocation (Single Job) ===\n";
    int page_size = get_int_input("Enter page size (bytes): ");
    long long job_size = get_ll_input("Enter job size (bytes): ");
    int num_frames = get_int_input("Enter number of physical frames in memory: ");
    if (page_size <=0 || num_frames<=0 || job_size<0) {
        cout << "Invalid values.\n"; return;
    }

    int num_pages = (int)((job_size + page_size - 1) / page_size);
    long long last_page_used = job_size % page_size;
    long long internal_frag = (last_page_used == 0 || num_pages==0) ? 0 : (page_size - last_page_used);

    Job job(1, job_size, num_pages, internal_frag);
    job.page_table.assign(num_pages, -1);

    vector<PageRef> frames(num_frames, PageRef());

    // Randomly load pages into frames 
    vector<int> pages(num_pages);
    iota(pages.begin(), pages.end(), 0);
    shuffle(pages.begin(), pages.end(), rng);
    int loaded = 0;
    for (int p : pages) {
        // find a free frame
        int free_idx = -1;
        for (int i=0;i<num_frames;++i) if (frames[i].job_id == -1) { free_idx = i; break; }
        if (free_idx == -1) break;
        frames[free_idx] = PageRef(job.id, p);
        job.page_table[p] = free_idx;
        ++loaded;
    }

    // Output summary
    cout << "\nJob summary:\n";
    cout << " Job id: 1\n";
    cout << " Job size: " << job_size << " bytes\n";
    cout << " Page size: " << page_size << " bytes\n";
    cout << " Number of pages required: " << num_pages << "\n";
    cout << " Internal fragmentation (in last page): " << internal_frag << " bytes\n";
    cout << " Pages loaded into memory: " << loaded << " / " << num_pages << "\n";

    show_frames(frames, page_size);

    // Allow address resolution queries
    while (true) {
        cout << "Resolve address? (y/n): ";
        char c; cin >> c;
        if (c!='y' && c!='Y') break;
        long long logical_addr = get_ll_input("Enter logical address (byte offset from job start): ");
        if (logical_addr < 0 || logical_addr >= job_size) {
            cout << "Logical address out of range (0 .. " << job_size-1 << ").\n";
            continue;
        }
        int page_no = (int)(logical_addr / page_size);
        int offset = (int)(logical_addr % page_size);
        int frame_no = job.page_table[page_no];
        if (frame_no == -1) {
            cout << "Page " << page_no << " is NOT loaded into memory. (No demand paging in this mode)\n";
        } else {
            long long physical_addr = (long long)frame_no * page_size + offset;
            cout << "Logical address " << logical_addr << " => Page " << page_no << ", Offset " << offset
                 << ". Physical frame " << frame_no << ". Physical address = " << physical_addr << ".\n";
        }
    }
    cout << "Exiting single-job paged mode.\n";
}

void mode_demand_multiple_jobs() {
    cout << "\n=== Demand Paged Memory Allocation (Multiple Jobs) ===\n";
    int page_size = get_int_input("Enter page size (bytes): ");
    int num_frames = get_int_input("Enter number of physical frames in memory: ");
    if (page_size <=0 || num_frames<=0) {
        cout << "Invalid values.\n"; return;
    }

    int job_count = get_int_input("How many jobs will you create? ");
    if (job_count <= 0) { cout << "No jobs to do.\n"; return; }

    vector<Job> jobs;
    jobs.reserve(job_count + 1);
    for (int i=1;i<=job_count;++i) {
        long long jsize = get_ll_input("Enter size for Job " + to_string(i) + " (bytes): ");
        if (jsize < 0) { cout << "Invalid size, setting to 0.\n"; jsize = 0; }
        int np = (int)((jsize + page_size - 1) / page_size);
        long long last_used = jsize % page_size;
        long long frag = (last_used == 0 || np==0) ? 0 : (page_size - last_used);
        Job job(i, jsize, np, frag);
        job.page_table.assign(np, -1);
        jobs.push_back(move(job));
    }

    vector<PageRef> frames(num_frames, PageRef());
    // frames initially free
    unordered_map<int,int> job_index; // job_id 
    for (int i=0;i<jobs.size();++i) job_index[jobs[i].id] = i;

    cout << "\nInitial state: all frames FREE.\n";
    show_frames(frames, page_size);

    // interactive loop: commands to load pages randomly
    while (true) {
        cout << "\nOptions:\n"
             << " 1) Randomly pre-load pages for a job (simulate initial random loading)\n"
             << " 2) Resolve logical address (may cause page fault & load)\n"
             << " 3) Show page tables\n"
             << " 4) Show frames\n"
             << " 5) Quit\n"
             << "Choose option: ";
        int opt; cin >> opt;
        if (opt == 1) {
            int jid = get_int_input("Job id to pre-load pages for: ");
            if (!job_index.count(jid)) { cout << "Job not found.\n"; continue; }
            Job &job = jobs[job_index[jid]];
            vector<int> pages(job.num_pages);
            iota(pages.begin(), pages.end(), 0);
            shuffle(pages.begin(), pages.end(), rng);
            int loaded=0;
            for (int p: pages) {
                // find free frame
                int free_idx=-1;
                for (int i=0;i<num_frames;++i) if (frames[i].job_id == -1) { free_idx = i; break; }
                if (free_idx == -1) break;
                frames[free_idx] = PageRef(job.id, p);
                job.page_table[p] = free_idx;
                loaded++;
            }
            cout << "Preloaded " << loaded << " pages for job " << jid << " (random assignment until memory full or job done).\n";
            show_frames(frames, page_size);
        } else if (opt == 2) {
            int jid = get_int_input("Enter job id for address resolution: ");
            if (!job_index.count(jid)) { cout << "Job not found.\n"; continue; }
            Job &job = jobs[job_index[jid]];
            long long logical_addr = get_ll_input("Enter logical address (byte offset from job start): ");
            if (logical_addr < 0 || logical_addr >= job.size) {
                cout << "Logical address out of range (0 .. " << max(0LL, job.size-1) << ").\n";
                continue;
            }
            int page_no = (int)(logical_addr / page_size);
            int offset = (int)(logical_addr % page_size);
            int frame_no = job.page_table[page_no];
            if (frame_no != -1) {
                long long physical_addr = (long long)frame_no * page_size + offset;
                cout << "Page present. Logical address " << logical_addr << " => Page " << page_no
                     << ", Offset " << offset << " -> Physical frame " << frame_no
                     << " -> Physical address " << physical_addr << ".\n";
            } else {
                cout << "Page fault: Page " << page_no << " of Job " << jid << " is not in memory.\n";
                // try to find free frame
                int free_idx = -1;
                for (int i=0;i<num_frames;++i) if (frames[i].job_id == -1) { free_idx = i; break; }
                if (free_idx != -1) {
                    cout << "Loading page into free frame " << free_idx << ".\n";
                    frames[free_idx] = PageRef(job.id, page_no);
                    job.page_table[page_no] = free_idx;
                } else {
                    cout << "No free frames. Evicting a random frame (random replacement).\n";
                    uniform_int_distribution<int> dist(0, num_frames-1);
                    int victim = dist(rng);
                    PageRef victimRef = frames[victim];
                    cout << " Evicting frame " << victim << ": Job " << victimRef.job_id << " Page " << victimRef.page_no << ".\n";
                    // update victim's page table
                    if (victimRef.job_id != -1) {
                        int vjid = victimRef.job_id;
                        int vpage = victimRef.page_no;
                        if (job_index.count(vjid)) {
                            jobs[job_index[vjid]].page_table[vpage] = -1;
                        }
                    }
                    // load new page
                    frames[victim] = PageRef(job.id, page_no);
                    job.page_table[page_no] = victim;
                    cout << " Loaded Job " << job.id << " Page " << page_no << " into frame " << victim << ".\n";
                }
                int new_frame = job.page_table[page_no];
                long long physical_addr = (long long)new_frame * page_size + offset;
                cout << "Now resolved: Physical frame " << new_frame << ", physical address = " << physical_addr << ".\n";
            }
        } else if (opt == 3) {
            cout << "\nPage tables (page -> frame or -1 if not loaded):\n";
            for (auto &job : jobs) {
                cout << " Job " << job.id << " (size " << job.size << " bytes, pages " << job.num_pages
                     << ", internal_frag " << job.internal_frag << "):\n  ";
                for (int p=0;p<job.num_pages;++p) {
                    cout << "[" << p << "->" << job.page_table[p] << "]";
                    if (p+1 < job.num_pages) cout << " ";
                }
                cout << "\n";
            }
        } else if (opt == 4) {
            show_frames(frames, page_size);
        } else if (opt == 5) {
            cout << "Quitting demand-paged simulation.\n";
            break;
        } else {
            cout << "Invalid option.\n";
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << "Paged Memory Simulation (C++)\n";
    while (true) {
        cout << "\nMain menu:\n 1) Paged Memory Allocation (single job, no demand paging)\n 2) Demand Paged Memory Allocation (multiple jobs)\n 3) Exit\n Choose an option: ";
        int opt;
        if (!(cin >> opt)) break;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (opt == 1) mode_paged_single_job();
        else if (opt == 2) mode_demand_multiple_jobs();
        else if (opt == 3) { cout << "Goodbye.\n"; break; }
        else cout << "Invalid option.\n";
    }
    return 0;
}
