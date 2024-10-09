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
    void start();
    void stop();

private:
    std::queue<std::shared_ptr<Process>> processQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> running;
    std::atomic<int> activeThreads;
    std::vector<std::thread> workerThreads;

    void run();
};

#endif // SCHEDULER_H
