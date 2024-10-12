#include "../include/Scheduler.h"

#include <iostream>

Scheduler::Scheduler() : running(false), activeThreads(0) {}

void Scheduler::addProcess(std::shared_ptr<Process> process)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    processQueue.push(process);
    queueCondition.notify_one();
}

// Setter methods
void Scheduler::setAlgorithm(const std::string& algorithm) {
    schedulerAlgo = algorithm;
}

void Scheduler::setNumCPUs(int num) {
    nCPU = num;
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
}

void Scheduler::run(int coreID)
{
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
            activeThreads++;
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(coreID);
            while (process->getCommandCounter() < process->getLinesOfCode())
            {
                process->executeCurrentCommand();
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_per_exec));
            }
            process->setProcess(Process::ProcessState::FINISHED);
            activeThreads--;
            queueCondition.notify_one();
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
            activeThreads++;
            process->setProcess(Process::ProcessState::RUNNING);
            process->setCPUCOREID(coreID);
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
            activeThreads--;
            queueCondition.notify_one();
        }
    }
}
