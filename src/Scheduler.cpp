#include "../include/Scheduler.h"
#include "../include/Process.h" 
#include "../include/CoreStateManager.h"

#include <iostream>
#include <chrono>
#include <iomanip>

Scheduler::Scheduler() : running(false), activeThreads(0), debugFile("debug.txt"), readyThreads(0) {}

void Scheduler::addProcess(std::shared_ptr<Process> process)
{
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
    //std::cout << "Initialized coreStates with size: " << CoreStateManager::getInstance().getCoreStates().size() << std::endl;
}

void Scheduler::setDelays(int delay) {
    delay_per_exec = delay;
}

void Scheduler::setQuantumCycle(int Quantum_cycle){
    quantum_cycle = Quantum_cycle;
}

void Scheduler::start()
{
    running = true;
    for (int i = 1; i <= nCPU; ++i)
    {
        workerThreads.emplace_back(&Scheduler::run, this, i);
    }
    // {
    //     std::unique_lock<std::mutex> lock(startMutex);
    //     startCondition.wait(lock, [this] { return readyThreads == nCPU; });
    // }
}

void Scheduler::stop()
{
    running = false;
    queueCondition.notify_all();
    for (auto &thread : workerThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    debugFile.close();
}

void Scheduler::run(int coreID)
{
    // {
    //     std::lock_guard<std::mutex> lock(startMutex);
    //     readyThreads++;
    //     if (readyThreads == nCPU) {
    //         startCondition.notify_one();
    //     }
    // }
    if (schedulerAlgo == "RR") {
        scheduleRR(coreID);
    } else if (schedulerAlgo == "FCFS") {
        scheduleFCFS(coreID);
    }
}

void Scheduler::scheduleFCFS(int coreID)
{
    while (running)
    {
        std::shared_ptr<Process> process;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]
            { return !processQueue.empty() || !running; });

            if (!running)
                break;

            process = processQueue.front();
            processQueue.pop();
        }

        if (process)
        {
            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads++;
                if (activeThreads > nCPU) {
                    std::cerr << "Error: Exceeded CPU limit!" << std::endl;
                    activeThreads--;
                    continue;
                }
            }
            logActiveThreads(coreID, process);
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(coreID);
            CoreStateManager::getInstance().setCoreState(coreID, true); // Mark core as in use
            while (process->getCommandCounter() < process->getLinesOfCode())
            {
                process->executeCurrentCommand();
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_per_exec));
            }
            process->setProcess(Process::ProcessState::FINISHED);
            
            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads--;
            }
            logActiveThreads(coreID, nullptr);
            queueCondition.notify_one();
        }
        else{
            CoreStateManager::getInstance().setCoreState(coreID, false); // Mark core as idle
        }
        
    }
}

void Scheduler::scheduleRR(int coreID)
{
    while (running)
    {
        std::shared_ptr<Process> process;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]
            { return !processQueue.empty() || !running; });

            if (!running)
                break;

            process = processQueue.front();
            processQueue.pop();
        }

        if (process)
        {
            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads++;
                if (activeThreads > nCPU) {
                    std::cerr << "Error: Exceeded CPU limit!" << std::endl;
                    activeThreads--;
                    continue;
                }
            }
            logActiveThreads(coreID, process);
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(coreID);
            CoreStateManager::getInstance().setCoreState(coreID, true); // Mark core as in use
            // std::cout << "Core " << coreID << " is used." << std::endl; // Debug statement
            int quantum = 0;
            while (process->getCommandCounter() < process->getLinesOfCode() && quantum < quantum_cycle)
            {
                process->executeCurrentCommand();
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_per_exec));
                quantum++;
            }
            if (process->getCommandCounter() < process->getLinesOfCode())
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                processQueue.push(process);
            }
            else
            {
                process->setProcess(Process::ProcessState::FINISHED);
            }
            
            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads--;
            }
            logActiveThreads(coreID, nullptr);
            queueCondition.notify_one();
        }
        else{
            std::cout << "ID: " << coreID << " :: " << "Processes: " << processQueue.size() << " ";
            CoreStateManager::getInstance().setCoreState(coreID, false); // Mark core as idle
        }
    }
}

void Scheduler::logActiveThreads(int coreID, std::shared_ptr<Process> currentProcess)
{
    std::lock_guard<std::mutex> lock(activeThreadsMutex);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    debugFile << "Timestamp: " << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count() << " ";
    debugFile << "Core ID: " << coreID << ", Active Threads: " << activeThreads << ", ";

    if (currentProcess) {
        debugFile << "Current Process: " << currentProcess->getPID() << ", ";
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
