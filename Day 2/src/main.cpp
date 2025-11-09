#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

int main() {
    std::string line, cmd, arg;
    std::cout << "=== File Explorer Day 2 ===\n";
    std::cout << "Commands: ls, cd <dir>, back, help, exit\n";

    while (true) {
        std::cout << "\"" << fs::current_path().string() << "\" > ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        iss >> cmd;

        if (cmd.empty()) continue;

        if (cmd == "exit") break;

        else if (cmd == "help") {
            std::cout << "Available commands:\n"
                      << "  ls            - list files and folders\n"
                      << "  cd <path>     - change directory\n"
                      << "  back          - go up one directory\n"
                      << "  help          - show this help menu\n"
                      << "  exit          - quit the program\n";
        }

        else if (cmd == "ls") {
            try {
                for (auto &p : fs::directory_iterator(fs::current_path()))
                    std::cout << p.path().filename().string() << "\n";
            } catch (const fs::filesystem_error &e) {
                std::cout << " Error: " << e.what() << "\n";
            }
        }

        else if (cmd == "cd") {
            std::getline(iss >> std::ws, arg);  // read full path (with spaces)
            if (!arg.empty() && arg.front() == '"' && arg.back() == '"')
                arg = arg.substr(1, arg.size() - 2);  // remove quotes if present

            try {
                fs::path target = fs::absolute(fs::path(arg));
                if (fs::exists(target) && fs::is_directory(target)) {
                    fs::current_path(target);
                } else if (fs::exists(fs::current_path() / arg)) {
                    fs::current_path(fs::current_path() / arg);
                } else {
                    std::cout << " Directory not found: " << arg << "\n";
                }
            } catch (const fs::filesystem_error &e) {
                std::cout << " Error: " << e.what() << "\n";
            }
        }

        else if (cmd == "back") {
            try {
                fs::current_path(fs::current_path().parent_path());
            } catch (const fs::filesystem_error &e) {
                std::cout << " Error: " << e.what() << "\n";
            }
        }

        else {
            std::cout << "  Unknown command. Type 'help' for a list of commands.\n";
        }
    }

    std::cout << " Exiting File Explorer...\n";
    return 0;
}



