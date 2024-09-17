#include <iostream>
#include <map>
#include <string>
#include <ctime>
#include <iomanip>


struct Session {
    std::string process_name;
    int current_line;
    int total_lines;
    std::string created_at;
};


std::map<std::string, Session> sessions;


void create_session(const std::string& name) {
    if (sessions.find(name) != sessions.end()) {
        std::cout << "Process '" << name << "' already exists...\n";
        return;
    }

    Session new_session;
    new_session.process_name = "Process-" + name;
    new_session.current_line = 0;
    new_session.total_lines = 100;

   
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(local_time, "%m/%d/%Y, %I:%M:%S %p");
    new_session.created_at = oss.str();

    sessions[name] = new_session;
    std::cout << "Screen '" << name << "' created.\n";
}


void display_session(const std::string& name) {
    auto it = sessions.find(name);
    if (it == sessions.end()) {
        std::cout << "Screen '" << name << "' does not exist.\n";
        return;
    }

    const Session& session = it->second;
    std::cout << "Process Name: " << session.process_name << "\n";
    std::cout << "Current Line: " << session.current_line << " / " << session.total_lines << "\n";
    std::cout << "Created At: " << session.created_at << "\n";
}


void main_menu() {
    while (true) {
        std::cout << "\nMain Menu:\n";
        std::cout << "Available Screens: ";
        for (const auto& pair : sessions) {
            std::cout << pair.first << " ";
        }
        std::cout << "\n";

        std::string command;
        std::cout << "$ ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }

        std::string cmd, option, name;
        std::istringstream iss(command);
        iss >> cmd >> option >> name;

        if (cmd == "screen" && (option == "-s" || option == "-r")) {
            if (option == "-s") {
                create_session(name);
            } else if (option == "-r") {
                display_session(name);
            }
        } else {
            std::cout << "Invalid command.\n";
        }
    }
}

int main() {
    main_menu();
    return 0;
}
