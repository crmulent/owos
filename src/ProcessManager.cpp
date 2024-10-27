#include "../include/ProcessManager.h"

ProcessManager::ProcessManager(int Min_ins, int Max_ins, int nCPU, std::string SchedulerAlgo, int delays_per_exec, int quantum_cycle, CPUClock* CpuClock)
{
    min_ins = Min_ins;
    max_ins = Max_ins;
    cpuClock = CpuClock;
    
    scheduler.setAlgorithm(SchedulerAlgo);
    scheduler.setDelays(delays_per_exec);
    scheduler.setNumCPUs(nCPU);
    scheduler.setQuantumCycle(quantum_cycle);
    scheduler.setCPUClock(CpuClock);

    schedulerThread = std::thread(&Scheduler::start, &scheduler);
}

void ProcessManager::addProcess(string name, string time)
{
    pid_counter++;
    shared_ptr<Process> process(new Process(pid_counter, name, time, -1, min_ins, max_ins));
    processList[name] = process;
    process->generate_commands(min_ins, max_ins);
    scheduler.addProcess(process);
}

shared_ptr<Process> ProcessManager::getProcess(string name)
{
    if(processList.find(name) != processList.end()){
        return processList[name];
    }else{
        return nullptr;
    }
    
}

map<string, std::shared_ptr<Process>> ProcessManager::getAllProcess()
{
    return processList;
}
