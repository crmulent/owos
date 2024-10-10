#include "../include/ConsoleManager.h"
#include "../include/General.h"

#include <iostream>
#include <cstdlib>

// Create a new screen session
void ConsoleManager::createSession(const std::string &name)
{
    if (screens.find(name) != screens.end())
    {
        std::cout << "Screen '" << name << "' already exists. Reattaching...\n";
        return;
    }

    Screen newScreen = {"Process-" + name, 0, 100, screenManager.getCurrentTimestamp()};
    screens[name] = newScreen;

    processManager.addProcess(name, screenManager.getCurrentTimestamp());

    std::cout << "Created screen: " << name << std::endl;
    clearscreen;
    processManager.getProcess(name);
    // screenManager.displayScreen(processManager.getProcess(name));
}

// Display all screens managed by ConsoleManager
void ConsoleManager::displayAllScreens()
{
    screenManager.displayAllProcess(processManager.getAllProcess());
}

// Handle user commands and delegate to appropriate functions
void ConsoleManager::handleCommand(const std::string &command)
{
    if (command == "initialize")
    {
        std::cout << "initialize command recognized. Doing something." << std::endl;
    }
    else if (command.rfind("screen -s ", 0) == 0)
    {
        std::string name = command.substr(10);

        for (int i = 0; i < 1000; i++) {
            createSession(name+to_string(i));
        }
    }
    else if (command.rfind("screen -r ", 0) == 0)
    {
        std::string name = command.substr(10);
        if (screens.find(name) != screens.end())
        {
            clearscreen;
        }
        else
        {
            std::cout << "No such screen exists." << std::endl;
        }
    }
    else if (command.rfind("screen -ls", 0) == 0)
    {
        displayAllScreens();
    }
    else if (command == "clear")
    {
        clearscreen;
        screenManager.displayHeader();
    }
    else if (command == "exit")
    {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }
    else
    {
        std::cout << "Unknown command. Please try again." << std::endl;
    }
}
