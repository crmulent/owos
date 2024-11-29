#ifndef FLAT_MEMORY_ALLOCATOR_H
#define FLAT_MEMORY_ALLOCATOR_H

#include <vector>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <list>
#include "IMemoryAllocator.h"

class FlatMemoryAllocator : public IMemoryAllocator {
public:
    FlatMemoryAllocator(size_t maximumSize, size_t mem_per_frame);
    ~FlatMemoryAllocator();

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
    size_t mem_per_frame;
    size_t allocatedSize;        // Currently allocated size
    std::vector<char> memory;    // Memory pool representation
    std::vector<bool> allocationMap;  // Allocation tracking map
    int nProcess;
    std::shared_mutex memoryMutex;
    std::map<size_t, std::shared_ptr<Process>> processList; //index of starting memory, name, size
    std::list<std::pair<size_t, size_t>> freeBlocks; // List of free blocks

    void initializeMemory();                      // Initializes memory and allocation map
    bool canAllocateAt(size_t index, size_t size) const;  // Checks if memory can be allocated at an index
    void allocateAt(size_t index, size_t size);   // Marks a block of memory as allocated
    void deallocateAt(size_t index, size_t size);              // Frees an allocated block of memory starting at index
    void logDeallocation(std::shared_ptr<Process> process); // Asynchronous logging
};

#endif // FLAT_MEMORY_ALLOCATOR_H