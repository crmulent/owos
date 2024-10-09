#include "ProcessManager.h"

// Add the implementation for addProcess
void ProcessManager::addProcess(string name, string time) {
    pid_counter++;
    shared_ptr<Process> process(new Process(pid_counter, name, time));
    processList[name] = process;
    process->generate_100_print_commands();
    process->executeCurrentCommand();
}

// Get a process by name
shared_ptr<Process> ProcessManager::getProcess(string name) {
    return processList[name];
}

// Get all processes
map<string, std::shared_ptr<Process>> ProcessManager::getAllProcess() {
    return processList;
}
