#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <fstream>
#include <string>

class Process; // Forward declaration

class Scheduler {
public:
    Scheduler();
    void addProcess(std::shared_ptr<Process> process);
    void setAlgorithm(const std::string& algorithm);
    void setNumCPUs(int num);
    void setDelays(int delay);
    void setQuantumCycle(int Quantum_cycle);
    void start();
    void stop();

private:
    void run(int coreID);
    void scheduleFCFS(int coreID);
    void scheduleRR(int coreID);
    void logActiveThreads(int coreID, std::shared_ptr<Process> currentProcess);

    bool running;
    int activeThreads;
    int nCPU;
    int delay_per_exec;
    int quantum_cycle;
    int readyThreads;
    std::string schedulerAlgo;
    std::queue<std::shared_ptr<Process>> processQueue;
    std::vector<std::thread> workerThreads;
    std::mutex queueMutex;
    std::mutex activeThreadsMutex;
    std::condition_variable queueCondition;
    std::ofstream debugFile;
    std::mutex startMutex;
    std::condition_variable startCondition;
};

#endif // SCHEDULER_H
