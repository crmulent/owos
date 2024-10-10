#ifndef PRINTCOMMAND_H
#define PRINTCOMMAND_H

#include "ICommand.h"

#include <ctime>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>

class PrintCommand : public ICommand
{
public:
    PrintCommand(int pid, int core, const std::string &toPrint, const std::string &Name)
        : ICommand(pid, CommandType::PRINT), Core(core), ToPrint(toPrint), name(Name) {}
    void execute() override
    {
        std::ofstream outfile(name + ".txt", std::ios::app);
        outfile << getCurrentTimestamp() << " Core:" << Core << " \"" << ToPrint << "\"" << std::endl;
        outfile.close();
    }
    void setCore(int core) override{
        Core = core;
    }

private:
    std::string ToPrint;
    int Core;
    std::string name;

    // This gets the current Timestamp when a process is created
    std::string getCurrentTimestamp()
    {
        std::time_t now = std::time(nullptr);
        std::tm local_time;
        localtime_s(&local_time, &now); // Use localtime_s for safety
        std::ostringstream oss;
        oss << std::put_time(&local_time, "(%m/%d/%Y %I:%M:%S%p) ");
        return oss.str();
    }
};

#endif // PRINTCOMMAND_H
