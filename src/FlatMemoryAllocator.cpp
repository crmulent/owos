#include "../include/FlatMemoryAllocator.h"
#include "../include/Process.h"
#include <iostream> 
#include <fstream>  
#include <ctime>  
#include <limits>  
#include <cstring>
#include <chrono>
#include <iomanip>  // For std::put_time
#include <memory>

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize, size_t mem_per_frame) 
    : maximumSize(maximumSize), allocatedSize(0), memory(maximumSize, '.'), allocationMap(maximumSize, false), mem_per_frame(mem_per_frame), nProcess(0) {
    initializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Ensure memory is safely cleared
    memory.clear();
    allocationMap.clear();
}

void* FlatMemoryAllocator::allocate(std::shared_ptr<Process> process) {
    size_t size = process->getMemoryRequired();
    
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex to prevent concurrent access
    for (size_t i = 0; i <= maximumSize - size; ++i) {
        if (canAllocateAt(i, size)) {
            allocateAt(i, size);
            nProcess++;
            processList[i] = process;
            return &memory[i];
        }
    }
    return nullptr;  // No sufficient contiguous block found
}

void FlatMemoryAllocator::deallocate(std::shared_ptr<Process> process) {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe deallocation

    size_t index = static_cast<char*>(process->getMemory()) - &memory[0];
    if (index < maximumSize && allocationMap[index]) {
        deallocateAt(index, process->getMemoryRequired());
        processList.erase(index);
        nProcess--;
    }
}

void FlatMemoryAllocator::visualizeMemory() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for consistent visualization
    std::cout << std::string(memory.begin(), memory.end());
}

void FlatMemoryAllocator::initializeMemory() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for initialization
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
    //std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for allocation
    for (size_t i = index; i < index + size; ++i) {
        allocationMap[i] = true;
        memory[i] = 'X';  // Mark allocated memory for visibility
    }
    allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index, size_t size) {
    //std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for deallocation
    for (size_t i = index; i < index + size; ++i) {
        allocationMap[i] = false;
        memory[i] = '.';  // Reset to initial state
    }
    allocatedSize -= size;
}

int FlatMemoryAllocator::getNProcess() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return nProcess;
}

std::map<size_t, std::shared_ptr<Process>> FlatMemoryAllocator::getProcessList() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return processList;
}

size_t FlatMemoryAllocator::getMaxMemory() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return maximumSize;
}

size_t FlatMemoryAllocator::getExternalFragmentation() {
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



void FlatMemoryAllocator::deallocateOldest(size_t memSize) {
    // Set the initial oldest time to the maximum possible time_point value
    std::chrono::time_point<std::chrono::system_clock> oldestTime = std::chrono::time_point<std::chrono::system_clock>::max();
    size_t oldestIndex = 0;  // To track the index of the oldest process
    std::shared_ptr<Process> oldestProcess = nullptr;  // To store the oldest process

    // Iterate through the process list to find the oldest process
    for (const auto &pair : processList) {
        size_t index = pair.first;
        std::shared_ptr<Process> process = pair.second;

        // Get the allocation time of the process
        std::chrono::time_point<std::chrono::system_clock> allocTime = process->getAllocTime();

        // Check if this process has the oldest allocation time and meets the memory requirements
        if (allocTime < oldestTime) {
            oldestTime = allocTime;
            oldestIndex = index;
            oldestProcess = process;
        }
    }

    // Now, oldestProcess holds the process with the oldest allocation time
    if (oldestProcess) {

        while(oldestProcess->getState() == Process::ProcessState::RUNNING){
            //wait until its not running
        }
        // Log the deallocation info to a backing store file
        std::ofstream backingStore("backingstore.txt", std::ios::app);  // Open file in append mode

        if (backingStore.is_open()) {
            // Convert the allocation time to a human-readable format
            std::time_t allocTime_t = std::chrono::system_clock::to_time_t(oldestProcess->getAllocTime());
            std::tm allocTime_tm = *std::localtime(&allocTime_t);

            // Write the deallocation log
            backingStore << "Deallocated Process ID: " << oldestProcess->getName() << "\n";
            backingStore << "Memory Deallocated: " << oldestProcess->getMemoryRequired() << " bytes\n";
            backingStore << "Deallocation Time: " << std::put_time(&allocTime_tm, "%Y-%m-%d %H:%M:%S") << "\n";
            backingStore << "============================================================================\n";

            backingStore.close();
        }
        
        if(oldestProcess->getState() != Process::ProcessState::FINISHED){
            // Perform the deallocation
            deallocate(oldestProcess);
            oldestProcess->setMemory(nullptr);
        }

    } else {
        std::cerr << "No process found to deallocate.\n";
    }
}


size_t FlatMemoryAllocator::getPageIn(){
    return 0;
}
size_t FlatMemoryAllocator::getPageOut(){
    return 0;
}
