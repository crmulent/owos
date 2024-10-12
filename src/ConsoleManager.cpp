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

    processManager->addProcess(name, screenManager.getCurrentTimestamp());

    std::cout << "Created screen: " << name << std::endl;
    clearscreen;
    processManager->getProcess(name);
    screenManager.displayScreen(processManager->getProcess(name));
}


// Create a new screen session
void ConsoleManager::generateSession(const std::string &name)
{
    if (screens.find(name) != screens.end())
    {
        std::cout << "Screen '" << name << "' already exists. Reattaching...\n";
        return;
    }

    Screen newScreen = {"Process-" + name, 0, 100, screenManager.getCurrentTimestamp()};
    screens[name] = newScreen;

    processManager->addProcess(name, screenManager.getCurrentTimestamp());

    // std::cout << "Created screen: " << name << std::endl;
    processManager->getProcess(name);
}
// Display all screens managed by ConsoleManager
void ConsoleManager::displayAllScreens()
{
    screenManager.displayAllProcess(processManager->getAllProcess(), nCPU);
}

// Handle user commands and delegate to appropriate functions
void ConsoleManager::handleCommand(const std::string &command)
{
    static bool schedulerRunning = false;
    static std::thread schedulerThread;

    if (command == "initialize")
    {
        std::ifstream config_file("config.txt");

    // Temporary variable to skip keys in the file
    std::string temp;

        if (config_file.is_open()) {
            // Read values in the expected order
            config_file >> temp >> nCPU;  // Read "num-cpu" and the value
            config_file >> temp >> std::quoted(scheduler);  // Read "scheduler" and the quoted value
            config_file >> temp >> quantum_cycles;  // Read "quantum-cycles" and the value
            config_file >> temp >> batch_process_freq;  // Read "batch-process-freq" and the value
            config_file >> temp >> min_ins;  // Read "min-ins" and the value
            config_file >> temp >> max_ins;  // Read "max-ins" and the value
            config_file >> temp >> delays_per_exec;  // Read "delays-per-exec" and the value

            config_file.close();  // Close the file after reading

            // Display the values
            std::cout << "num-cpu: " << nCPU << std::endl;
            std::cout << "scheduler: " << scheduler << std::endl;
            std::cout << "quantum-cycles: " << quantum_cycles << std::endl;
            std::cout << "batch-process-freq: " << batch_process_freq << std::endl;
            std::cout << "min-ins: " << min_ins << std::endl;
            std::cout << "max-ins: " << max_ins << std::endl;
            std::cout << "delays-per-exec: " << delays_per_exec << std::endl;

            processManager = new ProcessManager(min_ins, max_ins, nCPU, scheduler, delays_per_exec, quantum_cycles); 
        } else {
            std::cerr << "Unable to open file" << std::endl;
        }
    }
    else if (command.rfind("screen -s ", 0) == 0)
    {
        std::string name = command.substr(10);
        
        createSession(name);
    }
    else if (command.rfind("screen -r ", 0) == 0)
    {
        std::string name = command.substr(10);
        if (screens.find(name) != screens.end())
        {
            clearscreen;
            screenManager.displayScreen(processManager->getProcess(name));
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
    else if (command == "scheduler-test") {
        if (!schedulerRunning) {
            schedulerRunning = true;
            schedulerThread = std::thread([this]() {
                while (schedulerRunning) {
                    std::this_thread::sleep_for(std::chrono::seconds(batch_process_freq));
                    std::string name = "Process_" + std::to_string(screens.size());
                    generateSession(name);
                }
            });
        }
    }
    else if (command == "scheduler-stop") {
        schedulerRunning = false;
        if (schedulerThread.joinable()) {
            schedulerThread.join();
        }
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
