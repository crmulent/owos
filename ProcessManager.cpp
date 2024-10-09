#include "ProcessManager.h"

ProcessManager::ProcessManager() {
    scheduler.start();
}

void ProcessManager::addProcess(string name, string time) {
    pid_counter++;
    int core = pid_counter % 4; // Assign cores in a round-robin fashion (assuming 4 cores)
    shared_ptr<Process> process(new Process(pid_counter, name, time, core));
    processList[name] = process;
    process->generate_100_print_commands();
    scheduler.addProcess(process);
}

shared_ptr<Process> ProcessManager::getProcess(string name) {
    return processList[name];
}

map<string, std::shared_ptr<Process>> ProcessManager::getAllProcess() {
    return processList;
}
