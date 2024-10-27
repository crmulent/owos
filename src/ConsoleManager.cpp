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

//report-util functionality
void ConsoleManager::reportUtil()
{
    // Call the displayAllProcess method and capture the output
    std::stringstream output;
    screenManager.displayAllProcessToStream(processManager->getAllProcess(), nCPU, output);

    // Write the captured output to a file
    std::ofstream outFile("csopesy-log.txt");
    if (outFile.is_open())
    {
        outFile << output.str();
        outFile.close();
        std::cout << "Report saved to csopesy-log.txt" << std::endl;
    }
    else
    {
        std::cerr << "Unable to open file csopesy-log.txt" << std::endl;
    }
}

// Handle user commands and delegate to appropriate functions
void ConsoleManager::handleCommand(const std::string &command)
{
    static std::thread schedulerThread;


    if(!initialized && !(command == "exit" || command == "initialize")){
        std::cout << "[WARNING] Initialize the OS first using the \"initialize\" command\n";
        return;
    }


    if (command == "initialize")
    {
        clearscreen;
        screenManager.displayHeader();
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

            cpuClock = new CPUClock();
            cpuClock->startCPUClock();
            processManager = new ProcessManager(min_ins, max_ins, nCPU, scheduler, delays_per_exec, quantum_cycles, cpuClock); 

            initialized = true;

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
        std::shared_ptr<Process> process = processManager->getProcess(name);

        if (process != nullptr)
        {
            if(process->getState() != Process::FINISHED){
                clearscreen;
                screenManager.displayScreen(process);
            }else{
                std::cout << "Process " << name << " not found." <<std::endl;
            }

        }
        else
        {
            std::cout << "Process " << name << " not found." <<std::endl;
        }
    }
    else if (command.rfind("screen -ls", 0) == 0)
    {
        displayAllScreens();
    }
        
    //report-util command implementation
    else if (command == "report-util"){
        reportUtil();
    }
    else if (command == "scheduler-test") {
        if (!schedulerRunning) {
            schedulerRunning = true;
            std::cout << "Scheduler-test started\n";

            schedulerThread = std::thread([this]() {
                int tickCounter = 0;
                std::unique_lock<std::mutex> lock(cpuClock->getMutex(), std::defer_lock);

                while (schedulerRunning) {
                    // Wait for each CPU tick
                    lock.lock();
                    cpuClock->getCondition().wait(lock);
                    lock.unlock();

                    // Increment tick counter on each tick
                    tickCounter++;

                    // Generate session after every batch_process_freq ticks
                    if (tickCounter >= batch_process_freq) {
                        tickCounter = 0; // Reset counter
                        std::string name = "Process_" + std::to_string(screens.size());
                        generateSession(name);
                    }
                }
            });
        } else {
            std::cout << "[ERROR] \"scheduler-test\" command is already running\n";
        }
    }
    else if (command == "scheduler-stop") {
        if (schedulerRunning) {
            schedulerRunning = false;
            if (schedulerThread.joinable()) {
                schedulerThread.join();
                std::cout << "Scheduler-test stopped\n";
            }
        } else {
            std::cout << "[ERROR] \"scheduler-test\" is not running\n";
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
