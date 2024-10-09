#include "Scheduler.h"
#include <iostream>

Scheduler::Scheduler() : running(false), activeThreads(0) {}

void Scheduler::addProcess(std::shared_ptr<Process> process) {
    std::unique_lock<std::mutex> lock(queueMutex);
    processQueue.push(process);
    queueCondition.notify_one();
}

void Scheduler::start() {
    running = true;
    workerThreads.emplace_back(&Scheduler::run, this);
}

void Scheduler::stop() {
    running = false;
    queueCondition.notify_all();
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Scheduler::run() {
    while (running) {
        std::shared_ptr<Process> process;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return !processQueue.empty() || !running; });
            if (!running) break;
            if (activeThreads >= 4) {
                queueCondition.wait(lock, [this] { return activeThreads < 4 || !running; });
                if (!running) break;
            }
            process = processQueue.front();
            processQueue.pop();
        }

        if (process) {
            activeThreads++;
            std::thread processThread([this, process]() {
                std::cout << "Executing process: " << process->getName() << " on thread " << std::this_thread::get_id() << std::endl;
                while (process->getCommandCounter() < process->getLinesOfCode()) {
                    process->executeCurrentCommand();
                }
                std::cout << "Finished process: " << process->getName() << std::endl;
                activeThreads--;
                queueCondition.notify_one();
            });
            processThread.detach(); // Detach the thread to let it run independently
        }
    }
}
