#include "../include/Scheduler.h"
#include "../include/Process.h" 
#include "../include/CoreStateManager.h"
#include "../include/CPUClock.h"
#include "../include/FlatMemoryAllocator.h"

#include <iostream>
#include <chrono>
#include <iomanip>

Scheduler::Scheduler(std::string SchedulerAlgo, int delays_per_exec, int nCPU, int quantum_cycle, CPUClock* CpuClock, IMemoryAllocator* memoryAllocator) 
: running(false), activeThreads(0), debugFile("debug.txt"), readyThreads(0), schedulerAlgo(SchedulerAlgo), delay_per_exec(delays_per_exec)
, nCPU(nCPU), quantum_cycle(quantum_cycle), cpuClock(CpuClock), memoryAllocator(memoryAllocator){}


void Scheduler::addProcess(std::shared_ptr<Process> process) {
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

void Scheduler::stop() {
    running = false;
    queueCondition.notify_all();
    for (auto &thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    debugFile.close();
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

        // Find the first available core
        for (int i = 1; i <= nCPU; ++i) {
            if (!CoreStateManager::getInstance().getCoreState(i)) { // If core is not in use
                assignedCore = i;
                break; // Assign to the first available core
            }
        }

        if (assignedCore == -1) {
            // No core is available, process will be put back in the queue
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

            //logActiveThreads(assignedCore, process);
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(assignedCore);
            CoreStateManager::getInstance().setCoreState(assignedCore, true, process->getName()); // Mark core as in use

            int lastClock = cpuClock->getCPUClock();
            bool firstCommandExecuted = false;
            int cycleCounter = 0;

            while (process->getCommandCounter() < process->getLinesOfCode()) {
                {
                    // Wait for the next CPU cycle
                    std::unique_lock<std::mutex> lock(cpuClock->getMutex());
                    cpuClock->getCondition().wait(lock, [&] {
                        return cpuClock->getCPUClock() > lastClock;
                    });
                    lastClock = cpuClock->getCPUClock();
                }

                // Execute the first command immediately, then apply delay for subsequent commands
                if (!firstCommandExecuted || (++cycleCounter >= delay_per_exec)) {
                    process->executeCurrentCommand();
                    firstCommandExecuted = true;
                    cycleCounter = 0; // Reset cycle counter after each execution
                }
            }

            process->setProcess(Process::ProcessState::FINISHED);

            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads--;
            }
            //logActiveThreads(assignedCore, nullptr);
            queueCondition.notify_one();
        }

        CoreStateManager::getInstance().setCoreState(assignedCore, false, ""); // Mark core as idle
    }
}


void Scheduler::scheduleRR(int coreID)
{
    while (running) {
        std::shared_ptr<Process> process;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return !processQueue.empty() || !running; });

            if (!running)
                break;

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
           
            void* memory = memoryAllocator->allocate(process->getMemoryRequired(), process->getName());

            if (memory == nullptr) {
                //std::cerr << "Error: Memory Insufficient!" << std::endl;
                activeThreads--;

                // Return the process to the queue if memory allocation fails
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    processQueue.push(process);
                    queueCondition.notify_one();
                }
                continue;
            }
            
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(coreID);
            
            CoreStateManager::getInstance().setCoreState(coreID, true, process->getName()); // Mark core as in use
            
            int quantum = 0;
            int lastClock = cpuClock->getCPUClock();
            bool firstCommandExecuted = false;
            int cycleCounter = 0;

            while (process->getCommandCounter() < process->getLinesOfCode() && quantum < quantum_cycle) {
                if (delay_per_exec != 0)
                {
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
                    firstCommandExecuted = true;
                    cycleCounter = 0;
                    quantum++;
                }
            }

            {
                std::lock_guard<std::mutex> lock(logMutex);
                logMemoryState();  // Extracted log functionality for clarity
            }

            if (process->getCommandCounter() < process->getLinesOfCode()) {
                process->setProcess(Process::ProcessState::READY);
                std::lock_guard<std::mutex> lock(queueMutex);
                processQueue.push(process);
            } else {
                process->setProcess(Process::ProcessState::FINISHED);
            }

            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads--;
            }

            memoryAllocator->deallocate(memory, process->getMemoryRequired());
            queueCondition.notify_one();
        }

        CoreStateManager::getInstance().setCoreState(coreID, false, "");
    }
}





void Scheduler::logActiveThreads(int coreID, std::shared_ptr<Process> currentProcess)
{
    std::lock_guard<std::mutex> lock(activeThreadsMutex);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    struct tm timeinfo;
    localtime_s(&timeinfo, &now_c); // Use localtime_s instead of localtime

    debugFile << "Timestamp: " << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") << "." 
              << std::setfill('0') << std::setw(3) << ms.count() << " ";
    debugFile << "Core ID: " << coreID << ", Active Threads: " << activeThreads << ", ";

    if (currentProcess) {
        debugFile << "Current Process: " << currentProcess->getPID() << "(" 
                  << currentProcess->getCommandCounter() << "/" << currentProcess->getLinesOfCode() << "), ";
    } else {
        debugFile << "Current Process: None, ";
    }

    debugFile << "Ready Queue: ";
    std::unique_lock<std::mutex> queueLock(queueMutex); // Lock the queue mutex
    std::queue<std::shared_ptr<Process>> tempQueue = processQueue; // Copy the queue
    queueLock.unlock(); // Unlock the queue mutex

    while (!tempQueue.empty()) {
        debugFile << tempQueue.front()->getPID() << " ";
        tempQueue.pop();
    }
    debugFile << std::endl;
}

void Scheduler::logMemoryState() {
    std::ofstream outFile("memory_info.txt", std::ios::app);
    if (outFile.is_open()) {
        std::time_t currentTime = std::time(nullptr);
        char timestamp[100];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime));
        outFile << "Timestamp: " << timestamp << std::endl;

        outFile << "Number of processes in memory: "<< memoryAllocator->getNProcess() << std::endl;
        outFile << "Total external fragmentation in KB: " << memoryAllocator->getExternalFragmentation() << std::endl;
        outFile << "\n----end---- = "<< memoryAllocator->getMaxMemory() <<std::endl <<std::endl;

        std::map<size_t, std::tuple<std::string, size_t>> processList2 = memoryAllocator->getProcessList();
        for (const auto& pair : processList2) {
            size_t index = pair.first;
            const std::tuple<std::string, size_t>& value = pair.second;

            // Accessing the elements of the tuple
            const std::string& proc_name = std::get<0>(value);
            size_t size = std::get<1>(value);

            // Printing the values
            outFile << size <<std::endl;
            outFile << proc_name <<std::endl;
            outFile << index <<std::endl << std::endl;
        }
        outFile << "\n----start---- = 0"  <<std::endl;


        
        outFile.close();
    } else {
        std::cerr << "Unable to open the file!" << std::endl;
    }
}