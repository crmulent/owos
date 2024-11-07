#include "../include/ProcessManager.h"

ProcessManager::ProcessManager(int Min_ins, int Max_ins, int nCPU, std::string SchedulerAlgo, int delays_per_exec, int quantum_cycle, CPUClock* CpuClock
                                , size_t Max_mem, size_t Mem_per_frame, size_t Mem_per_proc)
{
    min_ins = Min_ins;
    max_ins = Max_ins;
    cpuClock = CpuClock;

    mem_per_proc = Mem_per_proc;
    max_mem = Max_mem;
    mem_per_frame = Mem_per_frame;
    
    memoryAllocator = new FlatMemoryAllocator(max_mem, mem_per_frame);

    scheduler = new Scheduler(SchedulerAlgo, delays_per_exec, nCPU, quantum_cycle, CpuClock, memoryAllocator);
    scheduler->setNumCPUs(nCPU);

    schedulerThread = std::thread(&Scheduler::start, scheduler);
}

void ProcessManager::addProcess(string name, string time)
{
    pid_counter++;
    shared_ptr<Process> process(new Process(pid_counter, name, time, -1, min_ins, max_ins, mem_per_proc));
    processList[name] = process;
    process->generate_commands(min_ins, max_ins);
    scheduler->addProcess(process);
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
