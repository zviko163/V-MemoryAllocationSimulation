# V-MemoryAllocationSimulation
Virtual Memory Allocation Simulation in C++

### Paged Memory Allocation (PMA.cpp)

This program simulates **paged memory allocation** using data loaded from a text file.  
It divides memory into page frames, allocates jobs, and reports internal fragmentation.

### Input Format
Default - input.txt

MemorySize <total_memory_KB> <page_size_KB>
<JobName> <job_size_KB>
<JobName2> <job_size_KB>
.
.
.

### Example

MemorySize 1024 128
JobA 200
JobB 300


### ▶Run
```bash
g++ PMA.cpp -o pma
./pma

The program outputs each job’s Page Map Table (PMT), Memory Map Table (MMT), and internal fragmentation details.

