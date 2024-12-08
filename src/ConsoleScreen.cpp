#include "../include/ConsoleScreen.h"
#include "../include/General.h"
#include "../include/CoreStateManager.h"

const char PINK[] = "\033[38;5;212m"; //test
const char GREEN[] = "\033[32m";
const char CYAN[] = "\033[36m";
const char RESET[] = "\033[0m";

// Display the header information on the console
void ConsoleScreen::displayHeader()
{
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

void ConsoleScreen::displayAllProcess(std::map<std::string, std::shared_ptr<Process>> processList, int nCore)
{
    displayAllProcessToStream(processList, nCore, std::cout);
}

void ConsoleScreen::displayAllProcessToStream(std::map<std::string, std::shared_ptr<Process>> processList, int nCore, std::ostream& out)
{
    static std::mutex processListMutex;  // Mutex for thread safety
    
    // Lock the mutex for the scope of this function
    std::lock_guard<std::mutex> lock(processListMutex);
    if (processList.empty())
    {
        out << "No screens available." << std::endl;
        return;
    }

    std::stringstream ready;
    std::stringstream running;
    std::stringstream finished;
    int coreUsage = 0;

    // Get core states from the CoreStateManager
    const std::vector<std::string>& run = CoreStateManager::getInstance().getProcess();
    const std::vector<bool>& coreStates = CoreStateManager::getInstance().getCoreStates();
    

    // Calculate core usage
    for (bool coreState : coreStates) {
        //std::cout << "CoreState: " << coreState << std::endl; //debugging
        if (coreState) {
            coreUsage++;
        }
    }

    // for (const std::string& name : run) {
    //     // Search for the process by name in the processList
    //     auto it = processList.find(name);
    //     if (it != processList.end()) {  // Check if process is found
    //         const std::shared_ptr<Process>& process = it->second;  // Access the shared_ptr
    //         std::stringstream temp;
    //         temp << std::left << std::setw(30) << process->getName()
    //             << " (" << process->getTime() << ") ";
    //         temp << "  Core: " << process->getCPUCoreID() << "   "
    //             << process->getCommandCounter() << " / "
    //             << process->getLinesOfCode() << std::endl;
    //         running << temp.str();  // Append to the running output
    //     }
    // }


    out << "Existing Screens:" << std::endl;
    for (const auto &pair : processList)
    {
        const std::shared_ptr<Process> process = pair.second;

        //construct the screen -ls
        std::stringstream temp;
        temp << std::left << std::setw(30) << process->getName() 
            << " (" << process->getTime() << ") ";
        
        // if(process->getState() == Process::READY){
        //     temp << "  READY " << "   "
        //     << process->getCommandCounter() << " / " 
        //     << process->getLinesOfCode() << std::endl;
        //     ready << temp.str() << std::endl;

        // }
        if (process->getState() == Process::RUNNING)
        {
            temp << "  Core: " << process->getCPUCoreID() << "   "
            << process->getCommandCounter() << " / " 
            << process->getLinesOfCode() << std::endl;
            running << temp.str() << std::endl;
        }
        if (process->getState() == Process::FINISHED)
        {
            temp << "  FINISHED " << "   "
            << process->getCommandCounter() << " / " 
            << process->getLinesOfCode() << std::endl;
            finished << temp.str() << std::endl;
        }
    }
    out << "CPU utilization: " << (static_cast<double>(coreUsage) / nCore) * 100 << "%\n";
    out << "Cores used: "<<coreUsage<< "\n";
    out << "Cores available: "<<nCore - coreUsage<< "\n";
    out << "------------------------------------------------\n";
    // out << "Ready Processes: \n"
    //     << ready.str();
    // std::cout << "==========================================\n";
    out << "\nRunning Processes: \n"
        << running.str();
    std::cout << "==========================================\n";
    out << "\nFinished Processes: \n"
        << finished.str();
    out << "------------------------------------------------\n";
}

void ConsoleScreen::displayUpdatedProcess(std::shared_ptr<Process> process)
{
    if (process->getState() == Process::RUNNING || process->getState() == Process::READY)
    {
        std::cout << CYAN << "Screen: " << process->getName() << RESET << std::endl;
        std::cout << "Current instruction line: " << process->getCommandCounter() << std::endl;
        std::cout << "Lines of code: " << process->getLinesOfCode() << std::endl;
        std::cout << std::endl;
    }
    else
    {
        std::cout << CYAN << "Screen: " << process->getName() << RESET << std::endl;
        std::cout << "Finished!" << std::endl;
        std::cout << std::endl;
    }
}

// Display a specific process
void ConsoleScreen::displayScreen(std::shared_ptr<Process> process)
{
    std::cout << CYAN << "Screen: " << process->getName() << RESET << std::endl;
    std::cout << "Instruction: Line " << process->getCommandCounter() << " / "
              << process->getLinesOfCode() << std::endl;
    std::cout << "Created at: " << process->getTime() << std::endl;
    std::cout << "Type 'exit' to return to the main menu." << std::endl;

    std::string command;
    while (true)
    {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        if (command == "process-smi") {
            displayUpdatedProcess(process);
        }
        else if (command == "exit")
        {
            clearscreen;
            displayHeader();
            break;
        }
        else
        {
            std::cout << "Unknown command. Please try again." << std::endl;
        }
    }
}

// Get the current timestamp when a process is created
std::string ConsoleScreen::getCurrentTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm local_time;
    localtime_s(&local_time, &now); // Use localtime_s for safety
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%m/%d/%Y, %I:%M:%S %p");
    return oss.str();
}
