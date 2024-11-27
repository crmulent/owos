#ifndef IMEMORYALLOCATOR_H
#define IMEMORYALLOCATOR_H

#include <vector>
#include <unordered_map>
#include <iostream>
#include <map>
#include <tuple>
#include "Process.h"

class IMemoryAllocator {
    public:
        virtual void* allocate(std::shared_ptr<Process> process) = 0;
        virtual void deallocate(std::shared_ptr<Process> process) = 0;
        virtual void visualizeMemory() = 0;
        virtual int getNProcess() = 0; 
        virtual std::map<size_t, std::shared_ptr<Process>>getProcessList() = 0;
        virtual size_t getMaxMemory() = 0;
        virtual size_t getExternalFragmentation() = 0;
        virtual void deallocateOldest(size_t memSize) = 0;
        virtual size_t getPageIn() = 0;
        virtual size_t getPageOut() = 0;
};
#endif // IMEMORYALLOCATOR_H