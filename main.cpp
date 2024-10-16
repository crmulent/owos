#include <iostream>
#include <cstdlib> 
#include <string>

// ANSI Escape Codes
const char GREEN[] = "\033[32m";
const char CYAN[] = "\033[36m";
const char PINK[] = "\e[38;5;212m";
const char RESET[] = "\033[0m";

void displayHeader();

int main() {
    std::string command;
    displayHeader();

    while (true) {

        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        if (command == "initialize") {
            std::cout << "initialize command recognized. Doing something." << std::endl;
        } else if (command == "screen") {
            std::cout << "screen command recognized. Doing something." << std::endl;
        } else if (command == "scheduler-test") {
            std::cout << "scheduler-test command recognized. Doing something." << std::endl;
        } else if (command == "scheduler-stop") {
            std::cout << "scheduler-stop command recognized. Doing something." << std::endl;
        } else if (command == "report-util") {
            std::cout << "report-util command recognized. Doing something." << std::endl;
        } else if (command == "clear") {
            #ifdef _WIN32
                system("CLS");
            #endif
            displayHeader();
        } else if (command == "exit") {
            std::cout << "Exiting..." << std::endl;
            break;
        } else {
            std::cout << "Unknown command. Please try again." << std::endl;
        }
    }

    return 0;
}

void displayHeader() {
std::cout << PINK << R"(
                                              ._.--._.
  /$$$$$$  /$$  /$$  /$$  /$$$$$$   /$$$$$$$  \|>_< |/
 /$$__  $$| $$ | $$ | $$ /$$__  $$ /$$_____/   |:_/ |
| $$  \ $$| $$ | $$ | $$| $$  \ $$|  $$$$$$   //    \ \   ?
| $$  | $$| $$ | $$ | $$| $$  | $$ \____  $$ (|      | ) /
|  $$$$$$/|  $$$$$/$$$$/|  $$$$$$/ /$$$$$$$/ /'\_   _/`\-
 \______/  \_____/\___/  \______/ |_______/  \___)=(___/
)" << RESET << std::endl;
std::cout << GREEN << "Hello, Welcome to OWOS commandline!" << RESET << std::endl;
std::cout << CYAN << "Type 'exit' to quit, 'clear' to clear screen" << RESET << std::endl;
}