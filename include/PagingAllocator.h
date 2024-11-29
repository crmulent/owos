#ifndef PAGING_ALLOCATOR_H
#define PAGING_ALLOCATOR_H

#include <vector>
#include <iostream>
#include "IMemoryAllocator.h"
#include <mutex>
#include <map>
#include <algorithm>

class PagingAllocator : public IMemoryAllocator {
public:
    PagingAllocator(size_t maximumSize, size_t mem_per_frame);

    void* allocate(std::shared_ptr<Process> process) override;
    void deallocate(std::shared_ptr<Process> process) override;
    void visualizeMemory() override;
    int getNProcess() override; 
    std::map<size_t, std::shared_ptr<Process>> getProcessList() override;
    size_t getMaxMemory() override;
    size_t getExternalFragmentation() override;
    void deallocateOldest(size_t memSize) override;
    size_t getPageIn() override;
    size_t getPageOut() override;

private:
    size_t maximumSize;          // Total size of the memory pool
    size_t numFrames;
    std::unordered_map<size_t, std::shared_ptr<Process>> frameMap;
    std::vector<size_t> freeFrameList;
    size_t nPagedIn;
    size_t nPagedOut;
    size_t mem_per_frame;
    size_t allocatedSize;        // Currently allocated size
    std::vector<char> memory;    // Memory pool representation
    std::vector<bool> allocationMap;  // Allocation tracking map
    int nProcess;
    std::mutex memoryMutex;
    std::map<size_t, std::shared_ptr<Process>> processList; //index of starting memory, name, size

    size_t allocateFrames(size_t numFrame, std::shared_ptr<Process> process);
    void deallocateFrames(size_t numFrame, size_t frameIndex);
};

#endif // PAGING_ALLOCATOR_H