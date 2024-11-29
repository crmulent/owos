#include "../include/Scheduler.h"
#include "../include/Process.h" 
#include "../include/CoreStateManager.h"
#include "../include/CPUClock.h"
#include "../include/FlatMemoryAllocator.h"

#include <iostream>
#include <chrono>
#include <iomanip>

Scheduler::Scheduler(std::string SchedulerAlgo, int delays_per_exec, int nCPU, int quantum_cycle, CPUClock* CpuClock, IMemoryAllocator* memoryAllocator) 
: running(false), activeThreads(0), readyThreads(0), schedulerAlgo(SchedulerAlgo), delay_per_exec(delays_per_exec)
, nCPU(nCPU), quantum_cycle(quantum_cycle), cpuClock(CpuClock), memoryAllocator(memoryAllocator){}

void Scheduler::addProcess(std::shared_ptr<Process> process) {
    if(!memoryLog){
        startMemoryLog();
    }

    std::unique_lock<std::mutex> lock(queueMutex);
    processQueue.push(process);
    queueCondition.notify_one();
}

void Scheduler::setAlgorithm(const std::string& algorithm) {
    schedulerAlgo = algorithm;
}

void Scheduler::setNumCPUs(int num) {
    nCPU = num;
    CoreStateManager::getInstance().initialize(nCPU);
}

void Scheduler::setDelays(int delay) {
    delay_per_exec = delay;
}

void Scheduler::setCPUClock(CPUClock* clock) {
    cpuClock = clock;  // Assign the member variable
}

void Scheduler::setQuantumCycle(int Quantum_cycle){
    quantum_cycle = Quantum_cycle;
}

void Scheduler::start() {
    running = true;
    for (int i = 1; i <= nCPU; ++i) {
        workerThreads.emplace_back(&Scheduler::run, this, i);
    }

    {
        std::unique_lock<std::mutex> lock(startMutex);
        startCondition.wait(lock, [this] { return readyThreads == nCPU; });
    }
}

void Scheduler::startMemoryLog() {
    memoryLog = true;
    memoryLoggingThread = std::thread([this]() {
        std::unique_lock<std::mutex> lock(cpuClock->getMutex());
        while (running) {
            cpuClock->getCondition().wait(lock);
            bool anyCoreActive = false;
            for (int i = 1; i <= nCPU; ++i) {
                if (CoreStateManager::getInstance().getCoreState(i)) {
                    anyCoreActive = true;
                    break;
                }
            }
            if (anyCoreActive) {
                cpuClock->incrementActiveCPUNum();
            }
        }
    });
}

void Scheduler::stop() {
    running = false;
    queueCondition.notify_all();

    if (memoryLoggingThread.joinable()) {
        memoryLoggingThread.join();
    }

    for (auto &thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Scheduler::run(int coreID) {
    {
        std::lock_guard<std::mutex> lock(startMutex);
        readyThreads++;
        if (readyThreads == nCPU) {
            startCondition.notify_one();
        }
    }
    if (schedulerAlgo == "rr") {
        scheduleRR(coreID);
    } else if (schedulerAlgo == "fcfs") {
        scheduleFCFS(coreID);
    }
}

void Scheduler::scheduleFCFS(int coreID)
{
    while (running) {
        std::shared_ptr<Process> process;
        int assignedCore = -1;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return !processQueue.empty() || !running; });

            if (!running)
                break;

            process = processQueue.front();
            processQueue.pop();
        }

        for (int i = 1; i <= nCPU; ++i) {
            if (!CoreStateManager::getInstance().getCoreState(i)) {
                assignedCore = i;
                break;
            }
        }

        if (assignedCore == -1) {
            std::unique_lock<std::mutex> lock(queueMutex);
            processQueue.push(process);
            continue;
        }

        if (process) {
            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads++;
                if (activeThreads > nCPU) {
                    std::cerr << "Error: Exceeded CPU limit!" << std::endl;
                    activeThreads--;
                    continue;
                }
            }

            void* memory = memoryAllocator->allocate(process);

            if(memory){
                process->setAllocTime();
                process->setMemory(memory);
            }

            if(!memory){
                do{
                    memoryAllocator->deallocateOldest(process->getMemoryRequired());
                    memory = memoryAllocator->allocate(process);
                    if(memory){
                        process->setAllocTime();
                        process->setMemory(memory);
                    }                        
                }while(!memory);
            }
            
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(assignedCore);
            CoreStateManager::getInstance().setCoreState(assignedCore, true, process->getName());

            int lastClock = cpuClock->getCPUClock();
            bool firstCommandExecuted = false;
            int cycleCounter = 0;

            while (process->getCommandCounter() < process->getLinesOfCode()) {
                {
                    std::unique_lock<std::mutex> lock(cpuClock->getMutex());
                    cpuClock->getCondition().wait(lock, [&] {
                        return cpuClock->getCPUClock() > lastClock;
                    });
                    lastClock = cpuClock->getCPUClock();
                }

                if (!firstCommandExecuted || (++cycleCounter >= delay_per_exec)) {
                    process->executeCurrentCommand();
                    firstCommandExecuted = true;
                    cycleCounter = 0;
                }

                // Introduce a delay to slow down CPU utilization
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            process->setProcess(Process::ProcessState::FINISHED);

            {
                memoryAllocator->deallocate(process);
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads--;
            }
            queueCondition.notify_one();
        }

        CoreStateManager::getInstance().setCoreState(assignedCore, false, "");
    }
}

void Scheduler::scheduleRR(int coreID)
{
    while (running) {
        std::shared_ptr<Process> process;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return !processQueue.empty() || !running; });

            if (!running) break;

            process = processQueue.front();
            processQueue.pop();
        }

        if (process) {
            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads++;
                if (activeThreads > nCPU) {
                    std::cerr << "Error: Exceeded CPU limit!" << std::endl;
                    activeThreads--;
                    continue;
                }
            }

            void* memory = process->getMemory();

            if (!memory) {
                memory = memoryAllocator->allocate(process);

                if(memory){
                    process->setAllocTime();
                    process->setMemory(memory);
                }

                if(!memory){
                    do{
                        memoryAllocator->deallocateOldest(process->getMemoryRequired());
                        memory = memoryAllocator->allocate(process);
                        if(memory){
                            process->setAllocTime();
                            process->setMemory(memory);
                        }                        
                    }while(!memory);
                }
            }
            
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(coreID);
            CoreStateManager::getInstance().setCoreState(coreID, true, process->getName());

            int quantum = 0;
            int lastClock = cpuClock->getCPUClock();
            bool firstCommandExecuted = true;
            int cycleCounter = 0;

            while (process->getCommandCounter() < process->getLinesOfCode() && quantum < quantum_cycle) {
                if (delay_per_exec != 0) {
                    std::unique_lock<std::mutex> lock(cpuClock->getMutex());
                    cpuClock->getCondition().wait(lock, [&] {
                        return cpuClock->getCPUClock() > lastClock;
                    });
                    lastClock = cpuClock->getCPUClock();
                } else {
                    std::this_thread::sleep_for(std::chrono::microseconds(1000));
                }

                if (!firstCommandExecuted || (++cycleCounter >= delay_per_exec)) {
                    process->executeCurrentCommand();
                    firstCommandExecuted = false;
                    cycleCounter = 0;
                    quantum++;
                }

                // Introduce a delay to slow down CPU utilization
                std::this_thread::sleep_for(std::chrono::milliseconds(0));
            }

            {
                std::lock_guard<std::mutex> lock(logMutex);
            }

            if (process->getCommandCounter() < process->getLinesOfCode()) {
                process->setProcess(Process::ProcessState::READY);
                std::lock_guard<std::mutex> lock(queueMutex);
                processQueue.push(process);
            } else {
                process->setProcess(Process::ProcessState::FINISHED);
                memoryAllocator->deallocate(process);
                process->setMemory(nullptr);
            }

            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads--;
            }

            queueCondition.notify_one();
        }

        CoreStateManager::getInstance().setCoreState(coreID, false, "");
    }
}