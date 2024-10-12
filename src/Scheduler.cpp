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
    while (running)
    {
        std::shared_ptr<Process> process;
        {
            //make sures that only one thread/core can access the ready queue
            std::unique_lock<std::mutex> lock(queueMutex);

            //wait until the queue is empty and the scheduler stops
            queueCondition.wait(lock, [this]
            { return !processQueue.empty() || !running; });

            if (!running)
                break;
            
            //gets the first process in the queue and pops it
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

                //put delay to analyze the scheduler
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_per_exec));
            }
            process->setProcess(Process::ProcessState::FINISHED);
            activeThreads--;
            queueCondition.notify_one();
        }
    }
}
