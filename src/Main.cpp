#include "../include/ConsoleManager.h"
#include "../include/ConsoleScreen.h"
#include "../include/General.h"

#include <map>
#include <ctime>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <iostream>

int main()
{
    ConsoleManager manager;
    ConsoleScreen screenManager;
    std::string command;

    clearscreen;
    screenManager.displayHeader();

    while (true)
    {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        if (command == "exit")
        {
            manager.handleCommand(command);
            break;
        }
        manager.handleCommand(command);
    }

    return 0;
}
