#include "../include/PagingAllocator.h"
#include "../include/Process.h"
#include <iostream> 
#include <fstream>  
#include <ctime>  
#include <limits>  
#include <cstring>
#include <chrono>
#include <iomanip>  // For std::put_time
#include <memory>
#include <algorithm>

PagingAllocator::PagingAllocator(size_t maximumSize, size_t mem_per_frame) 
    : maximumSize(maximumSize), 
      numFrames(static_cast<size_t>(std::ceil(static_cast<double>(maximumSize) / mem_per_frame))), 
      mem_per_frame(mem_per_frame), 
      nProcess(0), nPagedIn(0), nPagedOut(0) {

    for (size_t i = 0; i < numFrames; ++i) {
        freeFrameList.push_back(i);
    }
}

void* PagingAllocator::allocate(std::shared_ptr<Process> process) {
    std::lock_guard<std::mutex> lock(memoryMutex);
    size_t numFramesNeeded = process->getNumPages();
    if(numFramesNeeded > freeFrameList.size()){
        return nullptr;
    }
    
    size_t frameIndex = allocateFrames(numFramesNeeded, process);
    processList[process->getPID()] = process;
    nProcess++;
    return reinterpret_cast<void*>(frameIndex);
}

void PagingAllocator::deallocate(std::shared_ptr<Process> process) {
    std::lock_guard<std::mutex> lock(memoryMutex);
    processList.erase(process->getPID());    
    nProcess--;

    auto it = std::find_if(frameMap.begin(), frameMap.end(), [process](const auto& entry) {return entry.second == process;});

    while(it !=frameMap.end()){
        size_t frameIndex = it->first;
        deallocateFrames(1, frameIndex);
        it = std::find_if(frameMap.begin(), frameMap.end(), [process](const auto& entry) {return entry.second == process;});
    }

}


void PagingAllocator::visualizeMemory() {
    std::cout << "Memory Visualization:\n";

    for (size_t frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
        // Search for the frame in the map
        auto it = frameMap.find(frameIndex);

        if (it != frameMap.end()) {
            // If the frame is found, print the frame and the associated process
            std::cout << "Frame " << frameIndex << " -> Process " << it->second << "\n";
        } else {
            // If the frame is not found, it means it's free
            std::cout << "Frame " << frameIndex << " -> Free\n";
        }
    }
    std::cout << "---- End of memory visualization ----\n";
}


int PagingAllocator::getNProcess() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return nProcess;
}

std::map<size_t, std::shared_ptr<Process>> PagingAllocator::getProcessList() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return processList;
}

size_t PagingAllocator::getMaxMemory() {
    std::lock_guard<std::mutex> lock(memoryMutex);  // Lock mutex for thread-safe access
    return maximumSize;
}

size_t PagingAllocator::getExternalFragmentation() {
    std::lock_guard<std::mutex> lock(memoryMutex);
    return freeFrameList.size() * mem_per_frame;
}



void PagingAllocator::deallocateOldest(size_t memSize) {
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
            std::tm allocTime_tm;

            // Use localtime_s for thread safety
            if (localtime_s(&allocTime_tm, &allocTime_t) == 0) { // Check for success
                // Write the deallocation log
                backingStore << "Process ID: " << oldestProcess->getPID();
                backingStore << "  Name: " << oldestProcess->getName();
                backingStore << "  Command Counter: " << oldestProcess->getCommandCounter()
                    << "/" << oldestProcess->getLinesOfCode() << "\n";
                backingStore << "Memory Size: " << oldestProcess->getMemoryRequired() << " KB\n";
                backingStore << "Num Pages: " << oldestProcess->getNumPages() << "\n";
                backingStore << "============================================================================\n";

                backingStore.close();
            }
            else {
                // Handle the error if localtime_s fails
                std::cerr << "Failed to convert time to local time format." << std::endl;
            }
        }


        if(oldestProcess->getState() != Process::ProcessState::FINISHED){
            // Perform the deallocation
            deallocate(oldestProcess);
            oldestProcess->setMemory(nullptr);
        }

    }
}

size_t PagingAllocator::allocateFrames(size_t numFrame, std::shared_ptr<Process> process){
    size_t frameIndex = freeFrameList.back(); 
    // Map allocated frames to the process ID 
    for (size_t i = 0; i < numFrame; ++i) {
        freeFrameList.pop_back();
        frameMap [frameIndex + i] = process;
        nPagedIn++;
    }
    return frameIndex;
}

void PagingAllocator::deallocateFrames(size_t numFrame, size_t frameIndex){
    for (size_t i = 0 ; i < numFrame; ++i) { 
        frameMap.erase (frameIndex + i);
        nPagedOut++;
    }
    // Add frames back to the free frame list
    for (size_t i = 0; i < numFrame; ++i) {
       freeFrameList.push_back(frameIndex + i);
    }
}

size_t PagingAllocator::getPageIn(){
    std::lock_guard<std::mutex> lock(memoryMutex); 
    return nPagedIn;
}
size_t PagingAllocator::getPageOut(){
    std::lock_guard<std::mutex> lock(memoryMutex); 
    return nPagedOut;
}

