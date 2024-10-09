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

class PrintCommand : public ICommand{
public:
    PrintCommand(int pid, int core, const std::string& toPrint)
        : ICommand(pid, CommandType::PRINT), Core(core), ToPrint(toPrint) {
    }

    void execute() override{
        ofstream outfile (to_string(Pid) + ".txt", std::ios::app);
        outfile << ToPrint << std::endl;
        outfile.close();
    }
private:
    string ToPrint;
    int Core;
    int Pid;

    // This gets the current Timestamp when a process is created 
    string getCurrentTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm local_time;
        localtime_s(&local_time, &now);   // Use localtime_s for safety
        std::ostringstream oss;
        oss << std::put_time(&local_time, "%m/%d/%Y, %I:%M:%S %p");
        return oss.str();
    }
};


class Process {
public:
    struct RequirementFlags
    {
        bool requireFiles;
        int numFiles;
        bool requireMemory;
        int memoryRequired;
    };

    enum ProcessState{
        READY,
        RUNNING,
        WAITING,
        FINISHED
    };

    //Process(int pid, string name, RequirementFlags requirementFlags);
    Process(int pid, string name, string time){
        Pid = pid;
        Name = name;
        Time = time;
    }

    void addCommand(ICommand::CommandType commandType);
    void executeCurrentCommand() {
        CommandList[commandCounter]->execute();
        commandCounter++;
    }
    void moveToNextLine();
    bool isFinished() const;
    int getRemainingTime() const;

    int getCommandCounter(){
        return commandCounter;
    };

    int getLinesOfCode(){
        return CommandList.size();
    };

    int getCPUCoreID(){
        return cpuCoreID;
    }
    ProcessState getState() {
        return processState;
    }

    int getPID(){
        return Pid;
    }
    string getName(){
        return Name;
    }

    string getTime(){
        return Time;
    }

    void generate_100_print_commands(){
        for (int i = 1; i <= 100; ++i) {
            shared_ptr<ICommand> cmd = make_shared<PrintCommand>(Pid, cpuCoreID, "Hello World From  " + Name + " started.");
            CommandList.push_back(cmd); 
        }
    }


private:
    int Pid;
    string Name;
    string Time;
    vector<std::shared_ptr<ICommand>> CommandList;

    int commandCounter;
    int cpuCoreID = -1;
    RequirementFlags RequirementFlags;
    ProcessState processState;
};


