#include <iostream>
#include <vector>
#include <map>


using namespace std;

class ProcessManager{
private:
    map<string, std::shared_ptr<Process>> processList;
    int pid_counter = 0;


public: 
    void addProcess(string name, string time){
        pid_counter++;
        shared_ptr<Process> process(new Process(pid_counter, name, time));
        processList[name] = process;
        process->generate_100_print_commands();
        process->executeCurrentCommand();
    }

    shared_ptr<Process> getProcess(string name){
        return processList[name];
    }

    map<string, std::shared_ptr<Process>> getAllProcess(){
        return processList;
    }
};