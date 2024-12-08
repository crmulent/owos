#ifndef CONSOLES_SCREEN_H
#define CONSOLES_SCREEN_H

#include "Process.h"

#include <map>
#include <memory>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <mutex>

class ConsoleScreen
{
public:
    // Displays the header information on the console
    void displayHeader();

    // Displays all processes in the given map
    void displayAllProcess(std::map<std::string, std::shared_ptr<Process>> processList, int nCpu);

    // Displays updated process
    void displayUpdatedProcess(std::shared_ptr<Process> process);

    // Displays a specific process
    void displayScreen(std::shared_ptr<Process> process);

    void displayAllProcessToStream(std::map<std::string, std::shared_ptr<Process>> processList, int nCore, std::ostream& out);

    // Gets the current timestamp
    std::string getCurrentTimestamp();
    std::mutex processListMutex;
    std::mutex coreStatesMutex;
};

#endif // CONSOLES_SCREEN_H
