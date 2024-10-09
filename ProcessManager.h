#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "Process.h"
#include "Scheduler.h"

using namespace std;

class ProcessManager {
private:
    map<string, std::shared_ptr<Process>> processList; // Map of process names to shared pointers of Process
    int pid_counter = 0; // Counter for process IDs
    Scheduler scheduler; // Scheduler instance

public:
    ProcessManager();
    void addProcess(string name, string time);
    shared_ptr<Process> getProcess(string name);
    map<string, std::shared_ptr<Process>> getAllProcess();
};

#endif // PROCESSMANAGER_H
