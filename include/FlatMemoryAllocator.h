#ifndef FLAT_MEMORY_ALLOCATOR_H
#define FLAT_MEMORY_ALLOCATOR_H

#include <vector>
#include <iostream>
#include "IMemoryAllocator.h"
#include <mutex>
#include <map>
#include <tuple>

class FlatMemoryAllocator : public IMemoryAllocator {
public:
    FlatMemoryAllocator(size_t maximumSize, size_t mem_per_frame);
    ~FlatMemoryAllocator();

    void* allocate(size_t size, std::string processName) override;
    void deallocate(void* ptr, size_t size) override;
    std::string visualizeMemory() override;
    int getNProcess()override; 
    std::map<size_t, std::tuple<std::string, size_t>>getProcessList()override;
    size_t getMaxMemory()override;
    size_t getExternalFragmentation()override;

private:
    size_t maximumSize;          // Total size of the memory pool
    size_t mem_per_frame;
    size_t allocatedSize;        // Currently allocated size
    std::vector<char> memory;    // Memory pool representation
    std::vector<bool> allocationMap;  // Allocation tracking map
    int nProcess;

    void initializeMemory();                      // Initializes memory and allocation map
    bool canAllocateAt(size_t index, size_t size) const;  // Checks if memory can be allocated at an index
    void allocateAt(size_t index, size_t size);   // Marks a block of memory as allocated
    void deallocateAt(size_t index, size_t size);              // Frees an allocated block of memory starting at index
    std::mutex memoryMutex;
    std::map<size_t, std::tuple<std::string, size_t>> processList;
};

#endif // FLAT_MEMORY_ALLOCATOR_H
