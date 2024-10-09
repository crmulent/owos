#include <ctime>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>

using namespace std;

class ICommand
{
public:
    enum CommandType
    {
        PRINT
    };

    ICommand(int pid, CommandType commandType)
    {
        Pid = pid;
        this->commandType = commandType;
    }

    virtual void execute() = 0;

protected:
    int Pid;
    CommandType commandType;
};
