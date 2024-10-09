#ifndef PRINTCOMMAND_H
#define PRINTCOMMAND_H

#include "ICommand.h"
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>

class PrintCommand : public ICommand {
public:
    PrintCommand(int pid, int core, const std::string& toPrint)
        : ICommand(pid, CommandType::PRINT), Core(core), ToPrint(toPrint) {}

    void execute() override {
        std::ofstream outfile(std::to_string(Pid) + ".txt", std::ios::app);
        outfile << getCurrentTimestamp() << " Core:" << Core << " \"" << ToPrint << "\"" << std::endl;
        outfile.close();
    }

private:
    std::string ToPrint;
    int Core;

    // This gets the current Timestamp when a process is created
    std::string getCurrentTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm local_time;
        localtime_s(&local_time, &now); // Use localtime_s for safety
        std::ostringstream oss;
        oss << std::put_time(&local_time, "(%m/%d/%Y %I:%M:%S%p) ");
        return oss.str();
    }
};

#endif // PRINTCOMMAND_H