#include "../include/ProcessManager.h"

ProcessManager::ProcessManager(int Min_ins, int Max_ins, int nCPU, std::string SchedulerAlgo, int delays_per_exec, int quantum_cycle)
{
    min_ins = Min_ins;
    max_ins = Max_ins;
    scheduler.setAlgorithm(SchedulerAlgo);
    scheduler.setDelays(delays_per_exec);
    scheduler.setNumCPUs(nCPU);
    scheduler.setQuantumCycle(quantum_cycle);
    
    schedulerThread = std::thread(&Scheduler::start, &scheduler);
}

void ProcessManager::addProcess(string name, string time)
{
    pid_counter++;
    shared_ptr<Process> process(new Process(pid_counter, name, time, -1, min_ins, max_ins));
    processList[name] = process;
    process->generate_100_print_commands(min_ins, max_ins);
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
