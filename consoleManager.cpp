#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "ProcessManager.cpp"



// ConsoleManager class to store and manage console views (screens)
class ConsoleManager {
private:
    std::map<std::string, Screen> screens;
    ConsoleScreen screenManager;  // Uses ConsoleScreen for display operations
    ProcessManager processManager;

public:
    void createSession(const std::string& name);
    void displayAllScreens();
    void handleCommand(const std::string& command);
};

void ConsoleManager::createSession(const std::string& name) {
    if (screens.find(name) != screens.end()) {
        std::cout << "Screen '" << name << "' already exists. Reattaching...\n";
        return;
    }

    Screen newScreen = { "Process-" + name, 0, 100, screenManager.getCurrentTimestamp() };
    screens[name] = newScreen;

    processManager.addProcess(name, screenManager.getCurrentTimestamp());

    std::cout << "Created screen: " << name << std::endl;
    #ifdef _WIN32
        system("CLS");
    #endif
        processManager.getProcess(name);
        screenManager.displayScreen(processManager.getProcess(name));
}

void ConsoleManager::displayAllScreens() {
    screenManager.displayAllProcess(processManager.getAllProcess());
}

// This function handles user commands and delegates to ConsoleManager
void ConsoleManager::handleCommand(const std::string& command) {
    if (command == "initialize") {
        std::cout << "initialize command recognized. Doing something." << std::endl;
    } else if (command.rfind("screen -s ", 0) == 0) {
        std::string name = command.substr(10);
        createSession(name);


    } else if (command.rfind("screen -r ", 0) == 0) {


        std::string name = command.substr(10);
        if (screens.find(name) != screens.end()) {
            #ifdef _WIN32
                system("CLS");
            #endif
            //screenManager.displayScreen(screens[name]);
        } else {
            std::cout << "No such screen exists." << std::endl;
        }
    } else if (command.rfind("screen -ls", 0) == 0) {

        displayAllScreens();

    } else if (command == "clear") {
        #ifdef _WIN32
                system("CLS");
        #endif
        screenManager.displayHeader();
    } else if (command == "exit") {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    } else {
        std::cout << "Unknown command. Please try again." << std::endl;
    }
}