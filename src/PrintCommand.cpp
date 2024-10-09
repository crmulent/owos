#include "../include/ICommand.h"

#include <ctime>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>

using namespace std;

class PrintCommand : public ICommand
{
public:
    PrintCommand(int pid, int core, const std::string &toPrint)
        : ICommand(pid, CommandType::PRINT), Core(core), ToPrint(toPrint)
    {
    }

    void execute() override
    {
        ofstream outfile(to_string(Pid) + ".txt", std::ios::app);
        outfile << ToPrint << std::endl;
        outfile.close();
    }

private:
    string ToPrint;
    int Core;
    int Pid;

    // This gets the current Timestamp when a process is created
    string getCurrentTimestamp()
    {
        std::time_t now = std::time(nullptr);
        std::tm local_time;
        localtime_s(&local_time, &now); // Use localtime_s for safety
        std::ostringstream oss;
        oss << std::put_time(&local_time, "%m/%d/%Y, %I:%M:%S %p");
        return oss.str();
    }
};