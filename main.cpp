#include <iostream>
#include <cstdlib> 
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "consoleScreen.cpp"
#include "consoleManager.cpp"

// Main function
int main() {
    ConsoleManager manager;
    ConsoleScreen screenManager;
    std::string command;

    #ifdef _WIN32
        system("CLS");
    #endif
    
    screenManager.displayHeader();

    while (true) {

        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        manager.handleCommand(command);
    }

    return 0;
}
