#ifndef PRINTCOMMAND_H
#define PRINTCOMMAND_H

#include "ICommand.h"

#include <ctime>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>

class PrintCommand : public ICommand
{
public:
    PrintCommand(int pid, int core, const std::string &toPrint, const std::string &Name)
        : ICommand(pid, CommandType::PRINT), Core(core), ToPrint(toPrint), name(Name) {}
    void execute() override
    {
        // std::ofstream outfile(name + ".txt", std::ios::app);
        // outfile << getCurrentTimestamp() << " Core:" << Core << " \"" << ToPrint << "\"" << std::endl;
        // outfile.close();
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
        auto now = std::chrono::system_clock::now();
        std::time_t time_now = std::chrono::system_clock::to_time_t(now);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm local_time;
        localtime_s(&local_time, &time_now);
        std::ostringstream oss;
        oss << std::put_time(&local_time, "(%m/%d/%Y %I:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << milliseconds.count()
            << std::put_time(&local_time, "%p)");
        return oss.str();
    }
};

#endif // PRINTCOMMAND_H
