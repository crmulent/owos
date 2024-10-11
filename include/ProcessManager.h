#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "Process.h"
#include "Scheduler.h"

#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include <thread>

using namespace std;

class ProcessManager
{
private:
    map<string, std::shared_ptr<Process>> processList; // Map of process names to shared pointers of Process
    int pid_counter = 0;                               // Counter for process IDs
    Scheduler scheduler;                               // Scheduler instance
    thread schedulerThread;
    
public:
    ProcessManager();
    void addProcess(string name, string time);
    shared_ptr<Process> getProcess(string name);
    map<string, std::shared_ptr<Process>> getAllProcess();
        ~ProcessManager() {
        // Ensure the scheduler is stopped before destruction
        if (schedulerThread.joinable()) {
            scheduler.stop();
            schedulerThread.join();  // Wait for the thread to finish
        }
    }

};

#endif // PROCESSMANAGER_H