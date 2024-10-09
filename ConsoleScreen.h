#ifndef CONSOLES_SCREEN_H
#define CONSOLES_SCREEN_H

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include "Process.h"

class ConsoleScreen {
public:
    // Displays the header information on the console
    void displayHeader();

    // Displays all processes in the given map
    void displayAllProcess(std::map<std::string, std::shared_ptr<Process>> processList);

    // Displays a specific process
    void displayScreen(std::shared_ptr<Process> process);

    // Gets the current timestamp
    std::string getCurrentTimestamp();
};

#endif // CONSOLES_SCREEN_H