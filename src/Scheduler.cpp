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
        
        // Start an infinite loop to monitor the CPU clock ticks and log memory
        while (running) {
            // Wait for the clock tick to increment
            cpuClock->getCondition().wait(lock);

            bool anyCoreActive = false;
            // Check if at least one CPU core is running
            for (int i = 1; i <= nCPU; ++i) {
                if (CoreStateManager::getInstance().getCoreState(i)) { // Check if the core is active
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

    // Stop memory logging thread
    if (memoryLoggingThread.joinable()) {
        memoryLoggingThread.join();
    }

    // Join worker threads
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

            void* memory = memoryAllocator->allocate(process);

            //if allocation was succesful
            if(memory){
                process->setAllocTime();
                process->setMemory(memory);
            }

            //no free memory, replace some
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
                memoryAllocator->deallocate(process);
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
            // Minimize lock time by immediately checking if the queue is empty and only waiting if necessary
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

            // Check if the process already has memory allocated
            void* memory = process->getMemory();


            if (!memory) {
                memory = memoryAllocator->allocate(process);

                //if allocation was succesful
                if(memory){
                    process->setAllocTime();
                    process->setMemory(memory);
                }

                //no free memory, replace some
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
            CoreStateManager::getInstance().setCoreState(coreID, true, process->getName()); // Mark core as in use

            int quantum = 0;
            int lastClock = cpuClock->getCPUClock();
            bool firstCommandExecuted = true;
            int cycleCounter = 0;
            //std::this_thread::sleep_for(std::chrono::microseconds(2000));

            while (process->getCommandCounter() < process->getLinesOfCode() && quantum < quantum_cycle) {
                // Efficient wait mechanism; prevent unnecessary busy-waiting
                if (delay_per_exec != 0) {
                    std::unique_lock<std::mutex> lock(cpuClock->getMutex());
                    cpuClock->getCondition().wait(lock, [&] {
                        return cpuClock->getCPUClock() > lastClock;
                    });
                    lastClock = cpuClock->getCPUClock();
                } else {
                    //std::this_thread::sleep_for(std::chrono::microseconds(1000)); // Continue on if no delay is needed
                }



                // Execute the process commands with delay handling
                if (!firstCommandExecuted || (++cycleCounter >= delay_per_exec)) {
                    process->executeCurrentCommand();
                    firstCommandExecuted = false;
                    cycleCounter = 0;
                    quantum++;
                }
            }

            // Log memory state after each execution cycle
            {
                std::lock_guard<std::mutex> lock(logMutex);
            }
            std::this_thread::sleep_for(std::chrono::microseconds(2000));

            // If the process hasn't finished, move it back to the ready queue
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

        // Mark the core as idle after processing
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

void Scheduler::logMemoryState(int n) {
    // Generate filename with the current memory log cycle counter
    std::string filename = "generated_files/memory_stamp_" + std::to_string(n) + ".txt";
    std::ofstream outFile(filename);

    if (outFile.is_open()) {
        std::time_t currentTime = std::time(nullptr);
        char timestamp[100];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime));
        outFile << "Timestamp: (" << timestamp << ")" << std::endl;

        outFile << "Number of processes in memory: " << memoryAllocator->getNProcess() << std::endl;
        outFile << "Total external fragmentation in KB: " << memoryAllocator->getExternalFragmentation() << std::endl;
        outFile << "\n----end---- = " << memoryAllocator->getMaxMemory() << std::endl << std::endl;

        std::map<size_t, std::shared_ptr<Process>> processList2 = memoryAllocator->getProcessList();
        
        // Iterate in reverse order to match the display format
        for (auto it = processList2.rbegin(); it != processList2.rend(); ++it) {
            const auto& pair = *it;
        
            size_t index = pair.first;
            std::shared_ptr<Process> process = pair.second;

            // Accessing the elements of the tuple
            size_t size = process->getMemoryRequired();
            const std::string& proc_name = process->getName();

            // Printing the values
            outFile << size << std::endl;
            outFile << proc_name << std::endl;
            outFile << index << std::endl << std::endl;
        }

        outFile << "----start---- = 0" << std::endl;

        outFile.close();
    } else {
        std::cerr << "Unable to open the file!" << std::endl;
    }
}
