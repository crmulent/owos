#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>  
#include <sstream>
#include <ctime>
#include <iomanip>  // For std::put_time


using namespace std;

class ICommand{
public:
    enum CommandType{
        PRINT
    };

    ICommand (int pid, CommandType commandType){
        Pid = pid;
        this->commandType = commandType;
    }

    virtual void execute() = 0;
protected:
    int Pid;
    CommandType commandType;
};

