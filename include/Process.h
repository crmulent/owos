#ifndef PROCESS_H
#define PROCESS_H

#include "ICommand.h"
#include "PrintCommand.h"

#include <memory>
#include <string>
#include <vector>

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
    Process(int pid, const std::string &name, const std::string &time, int core);

    // Method to execute the current command
    void executeCurrentCommand();

    // Getters
    int getCommandCounter() const;
    int getLinesOfCode() const;
    int getCPUCoreID() const;
    void setCPUCOREID(int core);
    ProcessState getState() const;
    void setProcess(ProcessState state);
    int getPID() const;
    std::string getName() const;
    std::string getTime() const;

    // Method to generate print commands
    void generate_100_print_commands();

private:
    int Pid;
    std::string Name;
    std::string Time;
    std::vector<std::shared_ptr<ICommand>> CommandList;

    int commandCounter = 0;
    int cpuCoreID;
    RequirementFlags requirementFlags;
    ProcessState processState;
};

#endif // PROCESS_H