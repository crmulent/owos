#ifndef PROCESS_H
#define PROCESS_H

#include "ICommand.h"
#include "PrintCommand.h"

#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>

class Process
{
public:
    struct RequirementFlags
    {
        bool requireFiles;
        int numFiles;
        bool requireMemory;
        int memoryRequired;
    };

    enum ProcessState
    {
        READY,
        RUNNING,
        WAITING,
        FINISHED
    };

    // Constructor
    Process(int pid, const std::string &name, const std::string &time, int core, int minIns, int maxIns, size_t mem_per_proc, size_t mem_per_frame);

    // Method to execute the current command
    void executeCurrentCommand();

    // Getters
    int getCommandCounter() const;
    int getLinesOfCode() const;
    int getCPUCoreID() const;
    size_t getMemoryRequired() const;
    void setCPUCOREID(int core);
    ProcessState getState() const;
    void setProcess(ProcessState state);
    size_t getPID() const;
    std::string getName() const;
    std::string getTime() const;
    void setMemory(void* Memory);
    void* getMemory() const;
    void setAllocTime();
    std::chrono::time_point<std::chrono::system_clock> getAllocTime();
    size_t getNumPages();
    void calculateFrame();

    // Method to generate print commands
    void generate_commands(int minIns, int maxIns);

private:
    size_t Pid;
    std::string Name;
    std::string Time;
    std::vector<std::shared_ptr<ICommand>> CommandList;
    std::chrono::time_point<std::chrono::system_clock> allocationTime;  // Use chrono for allocation time


    size_t mem_per_proc;
    size_t mem_per_frame;
    size_t nPages;
    int commandCounter = 0;
    int cpuCoreID;
    RequirementFlags requirementFlags;
    ProcessState processState;
    void* memory;
};

#endif // PROCESS_H
