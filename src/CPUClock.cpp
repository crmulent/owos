#include "../include/CPUClock.h"

CPUClock::CPUClock() : cpuClock(0) {}

int CPUClock::getCPUClock() {
    return cpuClock.load();
}

void CPUClock::startCPUClock() {
    if (!isRunning) {
        isRunning = true;
        std::cout << "CPU Clock started\n";
        CPUClockThread = std::thread([this]() {
            while (isRunning) {
                {
                    std::lock_guard<std::mutex> lock(clockMutex);
                    cpuClock++;
                }
                
                cycleCondition.notify_all(); // Notify on each tick
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
               
            }
        });
    }
}

void CPUClock::stopCPUClock() {
    isRunning = false;
    if (CPUClockThread.joinable()) {
        CPUClockThread.join();
        std::cout << "CPU Clock stopped\n";
    }
}
