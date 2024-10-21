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
    {
        std::unique_lock<std::mutex> lock(startMutex);
        startCondition.wait(lock, [this] { return readyThreads == nCPU; });
    }
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
    while (running)
    {
        std::shared_ptr<Process> process;
        int assignedCore = -1;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]
            { return !processQueue.empty() || !running; });

            if (!running)
                break;

            process = processQueue.front();
            processQueue.pop();
        }

        // Find the first available core
        for (int i = 1; i <= nCPU; ++i)
        {
            if (!CoreStateManager::getInstance().getCoreState(i)) // If core is not in use
            {
                assignedCore = i;
                break; // Assign to the first available core
            }
        }

        if (assignedCore == -1)
        {
            // No core is available, process will be put back in the queue
            std::unique_lock<std::mutex> lock(queueMutex);
            processQueue.push(process);
            continue;
        }

        if (process)
        {
            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads++;
                if (activeThreads > nCPU)
                {
                    std::cerr << "Error: Exceeded CPU limit!" << std::endl;
                    activeThreads--;
                    continue;
                }
            }
            logActiveThreads(assignedCore, process);
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(assignedCore);
            CoreStateManager::getInstance().setCoreState(assignedCore, true); // Mark core as in use

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
            logActiveThreads(assignedCore, nullptr);
            queueCondition.notify_one();
        }
        CoreStateManager::getInstance().setCoreState(assignedCore, false); // Mark core as idle
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
                if (activeThreads > nCPU)
                {
                    std::cerr << "Error: Exceeded CPU limit!" << std::endl;
                    activeThreads--;
                    continue;
                }
            }

            // Log the current active threads and assign the process to this core
            logActiveThreads(coreID, process);
            process->setProcess(Process::ProcessState::RUNNING);  // Set process to RUNNING state
            process->setCPUCOREID(coreID);                        // Assign the current core (coreID) to this process
            CoreStateManager::getInstance().setCoreState(coreID, true); // Mark the core as in use

            // Execute the process for the time slice defined by quantum_cycle
            int quantum = 0;
            while (process->getCommandCounter() < process->getLinesOfCode() && quantum < quantum_cycle)
            {
                process->executeCurrentCommand();
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_per_exec));
                quantum++;
            }

            // If the process hasn't finished, push it back into the queue and mark it as READY
            if (process->getCommandCounter() < process->getLinesOfCode())
            {
                process->setProcess(Process::ProcessState::READY);  // Set process to READY state
                std::unique_lock<std::mutex> lock(queueMutex);
                processQueue.push(process);  // Put it back into the queue
            }
            else
            {
                process->setProcess(Process::ProcessState::FINISHED);  // Set process to FINISHED state
            }

            {
                std::lock_guard<std::mutex> lock(activeThreadsMutex);
                activeThreads--;
            }
            logActiveThreads(coreID, nullptr);
            queueCondition.notify_one();
        }

        CoreStateManager::getInstance().setCoreState(coreID, false); // Mark the core as idle
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
        debugFile << "Current Process: " << currentProcess->getPID() << "(" << currentProcess->getCommandCounter() << "/" 
            << currentProcess->getLinesOfCode() << "), ";
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
