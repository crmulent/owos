#include "../include/ProcessManager.h"

ProcessManager::ProcessManager()
{
    schedulerThread = std::thread(&Scheduler::start, &scheduler);
}

void ProcessManager::addProcess(string name, string time)
{
    pid_counter++;
    //int core = (pid_counter - 1) % 4; // Assign cores starting from 0 in a round-robin fashion (assuming 4 cores)
    shared_ptr<Process> process(new Process(pid_counter, name, time, -1));
    processList[name] = process;
    process->generate_100_print_commands();
    scheduler.addProcess(process);
}

shared_ptr<Process> ProcessManager::getProcess(string name)
{
    return processList[name];
}

map<string, std::shared_ptr<Process>> ProcessManager::getAllProcess()
{
    return processList;
}
