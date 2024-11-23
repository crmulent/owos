#include <vector>
#include <unordered_map>
#include <iostream>
#include <map>
#include <tuple>
#include "Process.h"

class IMemoryAllocator {
    public:
        virtual void* allocate(std::shared_ptr<Process> process) = 0;
        virtual void deallocate(void* prt, size_t size) = 0;
        virtual std::string visualizeMemory() = 0;
        virtual int getNProcess() = 0; 
        virtual std::map<size_t, std::shared_ptr<Process>>getProcessList() = 0;
        virtual size_t getMaxMemory() = 0;
        virtual size_t getExternalFragmentation() = 0;
        virtual void deallocateOldest(size_t memSize) = 0;
};