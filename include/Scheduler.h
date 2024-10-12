#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Process.h"

#include <mutex>
#include <queue>
#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>

class Scheduler
{
public:
    Scheduler();
    void addProcess(std::shared_ptr<Process> process);
    void setAlgorithm(const std::string& algorithm);
    void setDelays(int delay);
    void setNumCPUs(int num);
    void setQuantumCycle(int quantum_cycle);
    void start();
    void stop();

private:
    std::queue<std::shared_ptr<Process>> processQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> running;
    std::atomic<int> activeThreads;
    std::vector<std::thread> workerThreads;
    std::string schedulerAlgo;
    int nCPU;
    int delay_per_exec;
    int quantum_cycle;

    void run(int coreID);
};

#endif // SCHEDULER_H
