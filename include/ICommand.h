#ifndef ICOMMAND_H
#define ICOMMAND_H

#include <string>

class ICommand
{
public:
    enum CommandType
    {
        PRINT,
        // Add other command types here as needed
    };

    ICommand(int pid, CommandType commandType)
        : Pid(pid), commandType(commandType) {}

    virtual void execute() = 0;
    virtual void setCore(int core) = 0;

    

protected:
    int Pid;
    CommandType commandType;
};

#endif // ICOMMAND_H
