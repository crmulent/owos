#include "../include/FlatMemoryAllocator.h"
#include "../include/Process.h"
#include <iostream> 
#include <fstream>  
#include <ctime>  
#include <limits>  
#include <cstring>
#include <chrono>
#include <iomanip>  // For std::put_time
#include <future>
#include <sstream>

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize, size_t mem_per_frame) 
    : maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.'), allocationMap(maximumSize, false), mem_per_frame(mem_per_frame), nProcess(0) {
    initializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator() {
    std::unique_lock<std::shared_mutex> lock(memoryMutex);  // Ensure memory is safely cleared
    memory.clear();
    allocationMap.clear();
}

void* FlatMemoryAllocator::allocate(std::shared_ptr<Process> process) {
    size_t size = process->getMemoryRequired();
    
    std::unique_lock<std::shared_mutex> lock(memoryMutex);  // Lock mutex to prevent concurrent access
    for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
        if (it->second >= size) {
            size_t index = it->first;
            allocateAt(index, size);
            nProcess++;
            processList[index] = process;
            return &memory[index];
        }
    }
    return nullptr;  // No sufficient contiguous block found
}

void FlatMemoryAllocator::deallocate(std::shared_ptr<Process> process) {
    std::unique_lock<std::shared_mutex> lock(memoryMutex);  // Lock mutex for thread-safe deallocation

    size_t index = static_cast<char*>(process->getMemory()) - &memory[0];
    if (index < maximumSize && allocationMap[index]) {
        deallocateAt(index, process->getMemoryRequired());
        processList.erase(index);
        nProcess--;
        logDeallocation(process);
    }
}

void FlatMemoryAllocator::visualizeMemory() {
    std::shared_lock<std::shared_mutex> lock(memoryMutex);  // Lock mutex for consistent visualization
    std::cout << std::string(memory.begin(), memory.end());
}

void FlatMemoryAllocator::initializeMemory() {
    std::unique_lock<std::shared_mutex> lock(memoryMutex);
    std::fill(memory.begin(), memory.end(), '.');
    std::fill(allocationMap.begin(), allocationMap.end(), false);
    freeBlocks.clear();
    freeBlocks.emplace_back(0, maximumSize);  // Entire memory is initially free
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
    for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
        if (it->first <= index && it->first + it->second >= index + size) {
            return true;
        }
    }
    return false;
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size) {
    for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
        if (it->first <= index && it->first + it->second >= index + size) {
            size_t blockStart = it->first;
            size_t blockSize = it->second;
            freeBlocks.erase(it);

            // Adjust the free block before and/or after the allocation
            if (blockStart < index) {
                freeBlocks.emplace_back(blockStart, index - blockStart);
            }
            if (index + size < blockStart + blockSize) {
                freeBlocks.emplace_back(index + size, (blockStart + blockSize) - (index + size));
            }

            // Mark the memory as allocated
            for (size_t i = index; i < index + size; ++i) {
                allocationMap[i] = true;
                memory[i] = 'X';
            }
            allocatedSize += size;
            break;
        }
    }
}

void FlatMemoryAllocator::deallocateAt(size_t index, size_t size) {
    // Mark the memory as free
    for (size_t i = index; i < index + size; ++i) {
        allocationMap[i] = false;
        memory[i] = '.';
    }
    allocatedSize -= size;

    // Merge the deallocated block with adjacent free blocks
    auto it = freeBlocks.begin();
    while (it != freeBlocks.end() && it->first + it->second < index) {
        ++it;
    }

    if (it != freeBlocks.end() && it->first + it->second == index) {
        it->second += size;
    } else {
        freeBlocks.insert(it, {index, size});
    }

    it = freeBlocks.begin();
    while (it != freeBlocks.end() && it->first + it->second < index + size) {
        ++it;
    }

    if (it != freeBlocks.end() && it->first == index + size) {
        freeBlocks.insert(it, {index, it->second + size});
        freeBlocks.erase(it);
    }
}

int FlatMemoryAllocator::getNProcess() {
    std::shared_lock<std::shared_mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return nProcess;
}

std::map<size_t, std::shared_ptr<Process>> FlatMemoryAllocator::getProcessList() {
    std::shared_lock<std::shared_mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return processList;
}

size_t FlatMemoryAllocator::getMaxMemory() {
    std::shared_lock<std::shared_mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return maximumSize;
}

size_t FlatMemoryAllocator::getExternalFragmentation() {
    std::shared_lock<std::shared_mutex> lock(memoryMutex);  // Lock mutex for thread-safe operation
    size_t totalFreeSpace = 0;
    for (const auto& block : freeBlocks) {
        totalFreeSpace += block.second;
    }
    return totalFreeSpace;
}

void FlatMemoryAllocator::deallocateOldest(size_t memSize) {
    std::unique_lock<std::shared_mutex> lock(memoryMutex);
    std::chrono::time_point<std::chrono::system_clock> oldestTime = std::chrono::time_point<std::chrono::system_clock>::max();
    size_t oldestIndex = 0;
    std::shared_ptr<Process> oldestProcess = nullptr;

    for (const auto &pair : processList) {
        size_t index = pair.first;
        std::shared_ptr<Process> process = pair.second;
        std::chrono::time_point<std::chrono::system_clock> allocTime = process->getAllocTime();
        if (allocTime < oldestTime) {
            oldestTime = allocTime;
            oldestIndex = index;
            oldestProcess = process;
        }
    }

    if (oldestProcess) {
        while (oldestProcess->getState() == Process::ProcessState::RUNNING) {
            // wait until its not running
        }
        deallocate(oldestProcess);
        oldestProcess->setMemory(nullptr);
    } else {
        std::cerr << "No process found to deallocate.\n";
    }
}

size_t FlatMemoryAllocator::getPageIn() {
    return 0;
}

size_t FlatMemoryAllocator::getPageOut() {
    return 0;
}

void FlatMemoryAllocator::logDeallocation(std::shared_ptr<Process> process) {
    std::ostringstream logStream;
    logStream << "Process ID: " << process->getPID() << "  Name: " << process->getName() << "\n";
    logStream << "Memory Size: " << process->getMemoryRequired() << " KB\n";
    logStream << "Num Pages: " << process->getNumPages() << "\n";
    logStream << "============================================================================\n";

    std::string logMessage = logStream.str();
    std::async(std::launch::async, [logMessage]() {
        std::ofstream backingStore("backingstore.txt", std::ios::app);
        if (backingStore.is_open()) {
            backingStore << logMessage;
            backingStore.close();
        }
    });
}