#include "../include/ProcessManager.h"
#include "../include/CoreStateManager.h"
#include <random>

ProcessManager::ProcessManager(int Min_ins, int Max_ins, int NCPU, std::string SchedulerAlgo, int delays_per_exec, int quantum_cycle, CPUClock* CpuClock
                                , size_t Max_mem, size_t Mem_per_frame, size_t Min_mem_per_proc, size_t Max_mem_per_proc)
{
    min_ins = Min_ins;
    max_ins = Max_ins;
    cpuClock = CpuClock;

    min_mem_per_proc = Min_mem_per_proc;
    max_mem_per_proc = Max_mem_per_proc;
    max_mem = Max_mem;
    mem_per_frame = Mem_per_frame;
    nCPU = NCPU;
    
    if(max_mem == mem_per_frame){
        memoryAllocator = new FlatMemoryAllocator(max_mem, mem_per_frame);
    }else{
        memoryAllocator = new PagingAllocator(max_mem, mem_per_frame);
    }
    

    scheduler = new Scheduler(SchedulerAlgo, delays_per_exec, NCPU, quantum_cycle, CpuClock, memoryAllocator);
    scheduler->setNumCPUs(NCPU);

    schedulerThread = std::thread(&Scheduler::start, scheduler);
}

void ProcessManager::addProcess(string name, string time)
{
    pid_counter++;
    shared_ptr<Process> process(new Process(pid_counter, name, time, -1, min_ins, max_ins, generate_memory(), mem_per_frame));
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


void ProcessManager::process_smi() {
    static std::mutex processListMutex; 
    std::stringstream running;
    size_t memory_usage = 0;

    //for loop of process and memory usage
    int coreUsage = 0;

    // Get core states from the CoreStateManager
    const std::vector<std::string>& run = CoreStateManager::getInstance().getProcess();
    const std::vector<bool>& coreStates = CoreStateManager::getInstance().getCoreStates();
    

    // Calculate core usage
    for (bool coreState : coreStates) {
        //std::cout << "CoreState: " << coreState << std::endl; //debugging
        if (coreState) {
            coreUsage++;
        }
    }

    //for whole memory allocation even though process is not running
    std::map<size_t, std::shared_ptr<Process>> processList2 = memoryAllocator->getProcessList();
    // Iterate in reverse order to match the display format
    for (auto it = processList2.rbegin(); it != processList2.rend(); ++it) {
        const auto& pair = *it;
        
        size_t index = pair.first;
        std::shared_ptr<Process> process = pair.second;

        // Accessing the elements of the tuple
        size_t size = process->getMemoryRequired();
        const std::string& proc_name = process->getName();

        std::stringstream temp;
        temp << std::left << std::setw(30) << proc_name << " ";
        memory_usage += size;
        temp << size << " KB" << endl;
        running << temp.str() << endl;
    }


    // // for running processes only
    // for (const auto &pair : processList)
    // {
    //     const std::shared_ptr<Process> process = pair.second;

    //     std::stringstream temp;
    //     temp << std::left << std::setw(30) << process->getName() << " ";
        
    //     if(process->getState() == Process::RUNNING){
    //         size_t memory_req = process->getMemoryRequired();
    //         memory_usage += memory_req;

    //         temp << memory_req << " KiB" << endl;
    //         running << temp.str() << endl;
    //     }
    // }

    std::cout << "--------------------------------------------\n";
    std::cout << "| PROCESS-SMI V01.00 Driver Version: 01.00 |\n";
    std::cout << "--------------------------------------------\n";
    

    std::cout << "CPU-Util: " << (static_cast<double>(coreUsage) / nCPU) * 100 << "%" <<endl;
    std::cout << "Memory Usage: " << memory_usage << "KB"<< " / " << max_mem << "KB" << endl;
    std::cout << "Memory Util: " << (static_cast<double>(memory_usage) / max_mem) * 100 << "%" << endl;
    
    std::cout << "============================================\n"; 
    std::cout << "Running processes and memory usage:\n";
    std::cout << "--------------------------------------------\n";
    
    std::cout << running.str();


    std::cout << "--------------------------------------------\n";


    //implement the functions here
}


size_t ProcessManager::generate_memory() {
    std::random_device rd; // Obtain a random seed
    std::mt19937 gen(rd()); // Use Mersenne Twister for randomness
    std::uniform_int_distribution<size_t> dist(min_mem_per_proc, max_mem_per_proc);
    return dist(gen);
}
    

void ProcessManager::vmstat(){
    static std::mutex processListMutex;
    std::cout << "==========================================" << endl; 
    std::cout << "total memory KB:  " << max_mem << endl;
    std::cout << "used memory KB:   " << max_mem - memoryAllocator->getExternalFragmentation() << endl;
    std::cout << "free memory KB:   " << memoryAllocator->getExternalFragmentation() << endl;
    std::cout << "idle cpu ticks:   " << cpuClock->getCPUClock() - cpuClock->getActiveCPUNum() << endl;
    std::cout << "active cpu ticks: " << cpuClock->getActiveCPUNum() << endl;
    std::cout << "total cpu ticks:  " << cpuClock->getCPUClock() << endl;
    std::cout << "num paged in:     "<< memoryAllocator->getPageIn() << endl;
    std::cout << "num paged out:    "<< memoryAllocator->getPageOut() << endl; 
    std::cout << "==========================================" << endl; 
}