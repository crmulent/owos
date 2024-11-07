#include "../include/FlatMemoryAllocator.h"
#include <iostream>  // For std::cout

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize, size_t mem_per_frame) 
    : maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.'), allocationMap(maximumSize, false), mem_per_frame(mem_per_frame), nProcess(0) {
    initializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator() {
    memory.clear();
    allocationMap.clear();
}

void* FlatMemoryAllocator::allocate(size_t size, std::string processName) {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex to prevent concurrent access
    for (size_t i = 0; i <= maximumSize - size; ++i) {
        if (canAllocateAt(i, size)) {
            allocateAt(i, size);
            nProcess++;
            processList[i] = std::make_tuple(processName, i+size);
            return &memory[i];
        }
    }
    return nullptr;  // No sufficient contiguous block found
}

void FlatMemoryAllocator::deallocate(void* ptr, size_t size) {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe deallocation
    size_t index = static_cast<char*>(ptr) - &memory[0];
    if (index < maximumSize && allocationMap[index]) {
        deallocateAt(index, size);
        processList.erase(index);
        nProcess--;
    }
}

std::string FlatMemoryAllocator::visualizeMemory() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for consistent visualization
    return std::string(memory.begin(), memory.end());
}

void FlatMemoryAllocator::initializeMemory() {
    std::fill(memory.begin(), memory.end(), '.');
    std::fill(allocationMap.begin(), allocationMap.end(), false);
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
    if (index + size > maximumSize) {
        return false;
    }
    for (size_t i = index; i < index + size; ++i) {
        if (allocationMap[i]) {
            return false;
        }
    }
    return true;
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size) {
    for (size_t i = index; i < index + size; ++i) {
        allocationMap[i] = true;
        memory[i] = 'X';  // Mark allocated memory for visibility
    }
    allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index, size_t size) {
    for (size_t i = index; i < index + size; ++i) {
        allocationMap[i] = false;
        memory[i] = '.';  // Reset to initial state
    }
    allocatedSize -= size;
}

int FlatMemoryAllocator::getNProcess() {
    return nProcess;
}

std::map<size_t, std::tuple<std::string, size_t>> FlatMemoryAllocator::getProcessList() {
    return processList;
}

size_t FlatMemoryAllocator::getMaxMemory(){
    return maximumSize;
}

size_t FlatMemoryAllocator::getExternalFragmentation(){
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe operation
    size_t totalFreeSpace = 0;
    size_t currentFreeBlockSize = 0;

    for (size_t i = 0; i < maximumSize; ++i) {
        if (!allocationMap[i]) {
            // Current memory cell is free, so we're in a free block
            currentFreeBlockSize++;
        } else {
            // When we encounter an allocated memory block, check if there was a free block before
            if (currentFreeBlockSize > 0) {
                totalFreeSpace += currentFreeBlockSize;  // Add the free block size to the total
                currentFreeBlockSize = 0;  // Reset the free block size counter
            }
        }
    }

    // If there is a free block at the end of the memory
    if (currentFreeBlockSize > 0) {
        totalFreeSpace += currentFreeBlockSize;
    }

    return totalFreeSpace;
}