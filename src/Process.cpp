#include "../include/Process.h"

// Constructor implementation
Process::Process(int pid, const std::string &name, const std::string &time, int core, int minIns, int maxIns, size_t mem_per_proc)
    : Pid(pid), Name(name), Time(time), cpuCoreID(core), processState(READY), mem_per_proc(mem_per_proc), memory(nullptr){}

// Method to execute the current command
void Process::executeCurrentCommand()
{
    if (commandCounter < CommandList.size())
    {
        CommandList[commandCounter]->setCore(cpuCoreID);
        CommandList[commandCounter]->execute();
        commandCounter++;
    }
}

// Getter for command counter
int Process::getCommandCounter() const
{
    return commandCounter;
}

// Getter for command counter
void Process::setMemory(void* Memory)
{
    memory = Memory;
}

void* Process::getMemory() const
{
    return memory;
}



// Getter for number of commands
int Process::getLinesOfCode() const
{
    return CommandList.size();
}

size_t Process::getMemoryRequired() const{
    return mem_per_proc;
}

// Getter for CPU core ID
int Process::getCPUCoreID() const
{
    return cpuCoreID;
}


void Process::setCPUCOREID(int core){
    cpuCoreID = core;
}

// Getter for process state
Process::ProcessState Process::getState() const
{
    return processState;
}

void Process::setProcess(ProcessState state){
    processState = state;
}

// Getter for PID
int Process::getPID() const
{
    return Pid;
}

// Getter for Name
std::string Process::getName() const
{
    return Name;
}

// Getter for Time
std::string Process::getTime() const
{
    return Time;
}

void Process::generate_commands(int minIns, int maxIns) {

    std::srand(static_cast<unsigned int>(std::time(nullptr)) + Pid);

    int numCommands = minIns + (std::rand() % (maxIns - minIns + 1));

    for (int i = 1; i <= numCommands; ++i) {
        std::shared_ptr<ICommand> cmd = std::make_shared<PrintCommand>(Pid, cpuCoreID, "Hello World From " + Name + " started.", Name);
        CommandList.push_back(cmd);
    }
}

void Process::setAllocTime() {
    allocationTime = std::chrono::system_clock::now();  // Use chrono's system clock
}

// Getter for allocation time
std::chrono::time_point<std::chrono::system_clock> Process::getAllocTime() {
    return allocationTime;  // Return chrono time_point
}