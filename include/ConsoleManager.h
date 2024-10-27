#ifndef CONSOLE_MANAGER_H
#define CONSOLE_MANAGER_H

#include "ProcessManager.h"
#include "ConsoleScreen.h"
#include "CPUClock.h"

#include <string>
#include <map>
#include <iostream>
#include <fstream>

// ConsoleManager class to store and manage console views (screens)
class ConsoleManager
{
    int nCPU;
    std::string scheduler;
    int  quantum_cycles;
    int batch_process_freq;
    int min_ins;
    int max_ins;
    int delays_per_exec;
    bool initialized = false;
    bool schedulerRunning = false;
    CPUClock* cpuClock;


private:
    // Structure for storing screen info
    struct Screen
    {
        std::string processName;
        int currentLine;
        int totalLines;
        std::string timestamp;
    };

    std::map<std::string, Screen> screens; // Store screens
    ConsoleScreen screenManager;           // Uses ConsoleScreen for display operations
    ProcessManager* processManager;         // Manages processes

public:
    void createSession(const std::string &name);
    void generateSession(const std::string &name);
    void displayAllScreens();
    void reportUtil();
    void handleCommand(const std::string &command);
};

#endif // CONSOLE_MANAGER_H
