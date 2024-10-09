#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "Process.h"  // Include the Process class header

using namespace std;

class ProcessManager {
private:
    map<string, std::shared_ptr<Process>> processList; // Map of process names to shared pointers of Process
    int pid_counter = 0; // Counter for process IDs

public:
    // Add a new process
    void addProcess(string name, string time);

    // Get a process by name
    shared_ptr<Process> getProcess(string name);

    // Get all processes
    map<string, std::shared_ptr<Process>> getAllProcess();
};

#endif // PROCESSMANAGER_H
