#include "Process.h"

// Constructor implementation
Process::Process(int pid, const std::string& name, const std::string& time)
    : Pid(pid), Name(name), Time(time), commandCounter(0), cpuCoreID(-1), processState(READY) {}

// Method to execute the current command
void Process::executeCurrentCommand() {
    if (commandCounter < CommandList.size()) {
        CommandList[commandCounter]->execute();
        commandCounter++;
    }
}

// Getter for command counter
int Process::getCommandCounter() const {
    return commandCounter;
}

// Getter for number of commands
int Process::getLinesOfCode() const {
    return CommandList.size();
}

// Getter for CPU core ID
int Process::getCPUCoreID() const {
    return cpuCoreID;
}

// Getter for process state
Process::ProcessState Process::getState() const {
    return processState;
}

// Getter for PID
int Process::getPID() const {
    return Pid;
}

// Getter for Name
std::string Process::getName() const {
    return Name;
}

// Getter for Time
std::string Process::getTime() const {
    return Time;
}

// Method to generate 100 print commands
void Process::generate_100_print_commands() {
    for (int i = 1; i <= 100; ++i) {
        std::shared_ptr<ICommand> cmd = std::make_shared<PrintCommand>(Pid, cpuCoreID, "Hello World From " + Name + " started.");
        CommandList.push_back(cmd); 
    }
}
