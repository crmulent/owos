#include "../include/ProcessManager.h"

ProcessManager::ProcessManager(int min_ins, int max_ins, int nCPU, std::string SchedulerAlgo, int delays_per_exec, int quantum_cycle)
{
    scheduler.setAlgorithm(SchedulerAlgo);
    scheduler.setDelays(delays_per_exec);
    scheduler.setNumCPUs(nCPU);
    
    schedulerThread = std::thread(&Scheduler::start, &scheduler);
}

void ProcessManager::addProcess(string name, string time)
{
    pid_counter++;
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
