#ifndef CONSOLE_MANAGER_H
#define CONSOLE_MANAGER_H

#include "ProcessManager.h"
#include "ConsoleScreen.h"

#include <string>
#include <map>

// ConsoleManager class to store and manage console views (screens)
class ConsoleManager {
private:
    // Structure for storing screen info
    struct Screen {
        std::string processName;
        int currentLine;
        int totalLines;
        std::string timestamp;
    };

    std::map<std::string, Screen> screens;  // Store screens
    ConsoleScreen screenManager;             // Uses ConsoleScreen for display operations
    ProcessManager processManager;           // Manages processes

public:
    void createSession(const std::string& name);
    void displayAllScreens();
    void handleCommand(const std::string& command);
};

#endif // CONSOLE_MANAGER_H
