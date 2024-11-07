#include <vector>
#include <unordered_map>
#include <iostream>
#include <map>
#include <tuple>

class IMemoryAllocator {
    public:
        virtual void* allocate(size_t size, std::string processName) = 0;
        virtual void deallocate(void* prt, size_t size) = 0;
        virtual std::string visualizeMemory() = 0;
        virtual int getNProcess() = 0; 
        virtual std::map<size_t, std::tuple<std::string, size_t>>getProcessList() = 0;
        virtual size_t getMaxMemory() = 0;
        virtual size_t getExternalFragmentation() = 0;
};