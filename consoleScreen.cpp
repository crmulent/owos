#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <memory>

#include "Process.cpp"


const char GREEN[] = "\033[32m";
const char CYAN[] = "\033[36m";
const char PINK[] = "\e[38;5;212m";
const char RESET[] = "\033[0m";


// Structure for storing screen info
struct Screen {
    std::string processName;
    int currentLine;
    int totalLines;
    std::string timestamp;
};

// ConsoleScreen class to handle screen display operations
class ConsoleScreen {
public:
    void displayHeader();
    // void displayScreen(const Screen& screen);
    void displayScreen(std::shared_ptr<Process> process);
    std::string getCurrentTimestamp();
    void displayAllProcess(map<string, std::shared_ptr<Process>> processList);
};

void ConsoleScreen::displayHeader() {
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

// void ConsoleScreen::displayScreen(const Screen& screen) {
//     std::cout << CYAN << "Screen: " << screen.processName << RESET << std::endl;
//     std::cout << "Instruction: Line " << screen.currentLine << " / " << screen.totalLines << std::endl;
//     std::cout << "Created at: " << screen.timestamp << std::endl;
//     std::cout << "Type 'exit' to return to the main menu." << std::endl;

//     std::string command;
//     while (true) {
//         std::cout << "Enter a command: ";
//         std::getline(std::cin, command);
//         if (command == "exit") {
//         #ifdef _WIN32
//                 system("CLS");
//         #endif
//             displayHeader();
//             break;
//         } else {
//             std::cout << "Unknown command. Please try again." << std::endl;
//         }
//     }
// }

void ConsoleScreen::displayAllProcess(map<string, std::shared_ptr<Process>> processList) {
    if (processList.empty()) {
        std::cout << "No screens available." << std::endl;
        return;
    }

    stringstream  running;
    stringstream  finished;

    std::cout << CYAN << "Existing Screens:" << RESET << std::endl;
    for (const auto& pair : processList) {
        const shared_ptr<Process> process = pair.second;
        stringstream  temp;
        temp << std::setw(20) << "Name: " << process->getName() 
                    << std::setw(25) << "    (" << process->getTime() << ")" 
                    << std::setw(7) << "    Core: " << process->getCPUCoreID() 
                    << "    " << std::setw(6) << process->getCommandCounter() << " / " 
                    << std::setw(6) << process->getLinesOfCode() 
                    << std::endl;


        if(process->getState() == Process::RUNNING){
            running << temp.str() << endl;
        } else{
            finished << temp.str() << endl;
        }
    }

    cout << "Running Processes: \n";
    cout << running.str();
    cout << "Finished Processes: \n";
    cout << finished.str();
}

void ConsoleScreen::displayScreen(shared_ptr<Process> process) {
    std::cout << CYAN << "Screen: " << process->getName() << RESET << std::endl;
    std::cout << "Instruction: Line " << process->getCommandCounter() << " / " << process->getLinesOfCode() << std::endl;
    std::cout << "Created at: " << process->getTime() << std::endl;
    std::cout << "Type 'exit' to return to the main menu." << std::endl;

    std::string command;
    while (true) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        if (command == "exit") {
        #ifdef _WIN32
                system("CLS");
        #endif
            displayHeader();
            break;
        } else {
            std::cout << "Unknown command. Please try again." << std::endl;
        }
    }
}

// This gets the current Timestamp when a process is created 
std::string ConsoleScreen::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm local_time;
    localtime_s(&local_time, &now);   // Use localtime_s for safety
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%m/%d/%Y, %I:%M:%S %p");
    return oss.str();
}