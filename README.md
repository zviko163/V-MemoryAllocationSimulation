# Virtual Memory Allocation Simulation

A comprehensive C++ simulation suite demonstrating different virtual memory management techniques including paged memory allocation, demand paging, and page replacement algorithms.

## 🚀 Quick Start

### Compilation Commands
```bash
# Compile all programs
g++ PMA.cpp -o pma                    # Paged Memory Allocation
g++ demand_paged.cpp -o dpma          # Demand Paging Simulation  
g++ virtual_memory.cpp -o vm          # Virtual Memory with Page Replacement

# Or with C++11 standard (recommended)
g++ -std=c++11 PMA.cpp -o pma
g++ -std=c++11 demand_paged.cpp -o dpma
g++ -std=c++11 virtual_memory.cpp -o vm
```

### Running the Programs
```bash
# Run Paged Memory Allocation
./pma
# Enter: input.txt

# Run Demand Paging Simulation
./dpma
# Enter: input.txt

# Run Virtual Memory with Page Replacement
./vm
# Enter: vm_input.txt
```

## Input File Formats

### 1. Paged Memory Allocation (PMA.cpp) - `input.txt`
```
# Memory configuration
MemorySize <total_memory_KB> <page_size_KB>

# Job definition
<JobName> <job_size_KB>
...

# Address resolution requests 
Address <JobName> <page_number> <offset>

```

**Example `input.txt`:**
```
# Memory configuration
MemorySize 1024 128

# Job definition
JobA 200


# Address resolution requests
Address JobA 0 50
Address JobD 0 10

```

### 2. Demand Paging Simulation (demand_paged.cpp) - `input.txt`
Uses the same format as PMA.cpp but simulates partial page loading.

### 3. Virtual Memory with Page Replacement (virtual_memory.cpp) - `vm_input.txt`
```
# Page replacement policy (0=FIFO, 1=LRU)
ReplacementPolicy <0_or_1>

# Memory configuration
MemorySize <total_memory_KB> <page_size_KB>

# Job definitions
<JobName> <job_size_KB>
...

# Address resolution requests
Address <JobName> <page_number> <offset>
...
```

**Example `vm_input.txt`:**
```
ReplacementPolicy 1
MemorySize 1024 128

JobA 200
JobB 300
JobC 700

Address JobA 0 50
Address JobA 1 20
Address JobB 2 30
Address JobC 3 40
Address JobA 1 70
```

## Expected Output Examples

### Paged Memory Allocation Output
```
Enter input filename: input.txt

Allocating job JobA (200 KB) needing 2 pages...
Job JobA allocated successfully.

Internal Fragmentation for job JobA: 56 KB


Memory Map Table (MMT):
Frame	Status		Job(Page)
0	Free		-
1	Used		-
2	Used		JobA(0)
3	Used		-
4	Used		JobA(1)
5	Free		-
6	Used		-
7	Used		-

Page Map Table (PMT) for JobA:
Page	Frame
0	2
1	4

Address Resolution Results:
Job: JobA, Page: 0, Offset: 50 → Physical Address: 306 KB (Frame 2)
Job: JobA, Page: 1, Offset: 100 → Physical Address: 612 KB (Frame 4)
```

### Demand Paging Output
```
Enter input filename: input.txt

Loading Job JobA (200 KB)...
Total Pages: 2
Pages loaded into memory: 2

Loading Job JobB (300 KB)...
Total Pages: 3
Pages loaded into memory: 2

Memory Map Table (MMT):
Frame	Status		Job(Page)
0	Free		-
1	Used		JobC(2)
2	Used		JobB(2)
3	Used		JobA(0)
4	Used		JobB(1)
5	Used		JobE(0)
6	Used		JobA(1)
7	Used		JobC(1)

Page Map Table (PMT) for JobA:
Page	Frame
0	3
1	6

Address Resolution Results:
Job: JobA | Page: 0 | Offset: 50 → Physical Address: 434 KB (Frame 3)
Page Fault! JobC Page 3 not in memory.
```

### Virtual Memory with Page Replacement Output
```
Enter input filename: vm_input.txt

Loading Job JobA (200 KB)...
Total Pages: 2

Memory Map Table (MMT):
Frame	Status		Job(Page)
0	Used		JobB(0)
1	Used		JobA(1)
2	Used		JobA(0)
3	Free		-
4	Used		JobC(2)
5	Free		-
6	Used		JobC(1)
7	Used		JobC(0)

Address Resolution with Page Replacement (LRU):
Page Hit: JobA Page 0
Job: JobA | Page: 0 | Offset: 50 → Physical Address: 306 KB (Frame 2)

Page Fault! JobB Page 2 not in memory.
Job: JobB | Page: 2 | Offset: 30 → Physical Address: 414 KB (Frame 3)
```

## High-Level Intuitions and Approach

### 1. Paged Memory Allocation (PMA.cpp)

**Core Concept:** Simulates traditional paged memory allocation where the entire job is loaded into memory at once.

**Key Intuitions:**
- **Memory Division:** Physical memory is divided into fixed-size frames (pages)
- **Job Allocation:** Each job is allocated a contiguous set of pages based on its size
- **Internal Fragmentation:** Occurs when job size is not a multiple of page size
- **Random Allocation:** Frames are allocated randomly to simulate real-world memory management

**Approach:**
1. Parse memory configuration and job definitions
2. Calculate required pages for each job using `ceil(job_size / page_size)`
3. Allocate frames randomly using shuffled frame indices
4. Calculate and report internal fragmentation
5. Resolve logical addresses to physical addresses

### 2. Demand Paging Simulation (demand_paged.cpp)

**Core Concept:** Simulates demand paging where only a subset of pages are loaded into memory initially.

**Key Intuitions:**
- **Partial Loading:** Only 50-100% of pages are loaded initially (simulating real demand paging)
- **Page Faults:** Accessing unloaded pages triggers page faults
- **Memory Efficiency:** Reduces initial memory usage but may cause performance issues
- **On-Demand Loading:** Pages are loaded only when needed

**Approach:**
1. Calculate total pages needed for each job
2. Randomly select 50-100% of pages to load initially
3. Simulate page faults when accessing unloaded pages
4. Display which pages are in memory vs. not in memory

### 3. Virtual Memory with Page Replacement (virtual_memory.cpp)

**Core Concept:** Advanced virtual memory management with page replacement algorithms (FIFO and LRU).

**Key Intuitions:**
- **Page Replacement:** When memory is full, existing pages must be evicted
- **FIFO (First-In-First-Out):** Replaces the oldest page in memory
- **LRU (Least Recently Used):** Replaces the least recently accessed page
- **Page Hit vs. Page Fault:** Distinguishes between pages already in memory and those needing loading
- **Dynamic Memory Management:** Handles memory allocation and deallocation dynamically

**Approach:**
1. Implement FIFO using a queue to track page insertion order
2. Implement LRU using a doubly-linked list to track access order
3. Handle page faults by finding free frames or replacing existing pages
4. Update replacement data structures on each memory access
5. Provide detailed logging of page hits, faults, and replacements

## Technical Implementation Details

### Data Structures
- **Frame:** Represents a physical memory frame with status and job information
- **Page:** Represents a logical page with frame mapping
- **Job:** Contains job metadata and page mappings
- **Memory:** Manages all frames and memory configuration
- **AddressRequest:** Represents logical address resolution requests

### Algorithms
- **Random Frame Allocation:** Uses `std::shuffle` for unbiased frame selection
- **FIFO Replacement:** Uses `std::queue` for O(1) insertion and removal
- **LRU Replacement:** Uses `std::list` for O(1) access and removal operations
- **Address Translation:** Converts logical addresses to physical addresses using page tables

### Memory Management Features
- **Internal Fragmentation Calculation:** Measures wasted space within pages
- **Page Fault Detection:** Identifies when pages are not in memory
- **Frame Status Tracking:** Monitors which frames are free or occupied
- **Address Resolution:** Converts logical addresses to physical addresses

## Educational Value

This simulation suite demonstrates:
- **Memory Management Concepts:** Paging, segmentation, virtual memory
- **Page Replacement Algorithms:** FIFO, LRU, and their trade-offs
- **Memory Fragmentation:** Internal fragmentation in paged systems
- **Address Translation:** Logical to physical address mapping
- **Performance Implications:** Page faults, memory efficiency, and replacement policies

