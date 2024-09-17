#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>

// ANSI Escape Codes
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

// Maps to store screen states
std::map<std::string, Screen> screens;

// Function declarations
void displayHeader();
void displayScreen(const Screen& screen);
std::string getCurrentTimestamp();
void createSession(const std::string& name);
void handleCommand(const std::string& command);

int main() {
    std::string command;
    displayHeader();

    while (true) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        handleCommand(command);
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

// This function handles a user commands
void handleCommand(const std::string& command) {
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
            displayScreen(screens[name]);
        } else {
            std::cout << "No such screen exists." << std::endl;
        }

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
        exit(0);

    } else {
        std::cout << "Unknown command. Please try again." << std::endl;
    }
}

// This function creates a new screen session
void createSession(const std::string& name) {
    if (screens.find(name) != screens.end()) {
        std::cout << "Screen '" << name << "' already exists. Reattaching...\n";
        return;
    }

    Screen newScreen = { "Process-" + name, 0, 100, getCurrentTimestamp() };
    screens[name] = newScreen;
    std::cout << "Created screen: " << name << std::endl;
}

// This function displays an existing screen's information
void displayScreen(const Screen& screen) {
    std::cout << CYAN << "Screen: " << screen.processName << RESET << std::endl;
    std::cout << "Instruction: Line " << screen.currentLine << " / " << screen.totalLines << std::endl;
    std::cout << "Created at: " << screen.timestamp << std::endl;
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
std::string getCurrentTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(now, "%m/%d/%Y, %I:%M:%S %p");
    return oss.str();
}
