#ifndef CPU_CLOCK_H
#define CPU_CLOCK_H
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <mutex>

class CPUClock {
public:
    CPUClock();
    int getCPUClock();
    void startCPUClock();
    void stopCPUClock();
    
    // Accessors to use condition variable and mutex externally
    std::condition_variable& getCondition() { return cycleCondition; }
    std::mutex& getMutex() { return clockMutex; }

private:
    std::atomic<int> cpuClock;
    bool isRunning = false;
    std::thread CPUClockThread;
    std::condition_variable cycleCondition;
    std::mutex clockMutex;
};
#endif
