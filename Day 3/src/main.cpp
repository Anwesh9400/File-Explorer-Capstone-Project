// Day3/src/main.cpp
#include "explorer.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

// Helper: convert perms → rwxr-xr--
std::string format_permissions(fs::perms p) {
    auto test = [&](fs::perms bit, char c) {
        return ((p & bit) != fs::perms::none) ? c : '-';
    };
    std::string s;
    s += test(fs::perms::owner_read, 'r');
    s += test(fs::perms::owner_write, 'w');
    s += test(fs::perms::owner_exec, 'x');
    s += test(fs::perms::group_read, 'r');
    s += test(fs::perms::group_write, 'w');
    s += test(fs::perms::group_exec, 'x');
    s += test(fs::perms::others_read, 'r');
    s += test(fs::perms::others_write, 'w');
    s += test(fs::perms::others_exec, 'x');
    return s;
}

void print_ls(const std::vector<Entry>& entries, const fs::path& base) {
    std::cout << std::left << std::setw(11) << "PERMS"
              << std::setw(10) << "SIZE"
              << "NAME\n";
    std::cout << "-------------------------------------\n";

    for (const auto& e : entries) {
        fs::path full = base / e.name;
        std::string perms = "---------";
        try { perms = format_permissions(fs::status(full).permissions()); } catch (...) {}
        std::string size = e.is_dir ? "<DIR>" : std::to_string(e.size);
        std::cout << std::left << std::setw(11) << perms
                  << std::setw(10) << size
                  << e.name << '\n';
    }
}

int main() {
    fs::path current = fs::current_path();
    std::string line;

    while (true) {
        std::cout << current.string() << " $ ";
        std::getline(std::cin, line);
        if (line.empty()) continue;

        if (line == "exit") break;
        else if (line == "pwd") {
            std::cout << current.string() << "\n";
        }
        else if (line == "ls") {
            auto entries = list_directory(current.string());
            print_ls(entries, current);
        }
        else if (line.rfind("cd ", 0) == 0) {
            std::string dir = line.substr(3);
            fs::path newp = (dir == "..") ? current.parent_path() : current / dir;
            if (fs::exists(newp) && fs::is_directory(newp))
                current = fs::canonical(newp);
            else
                std::cout << "No such directory\n";
        }

        // ---------- FILE OPERATIONS ----------
        else if (line.rfind("touch ", 0) == 0) {
            std::string fname = line.substr(6);
            std::ofstream(current / fname); // create empty file
            std::cout << "Created file: " << fname << "\n";
        }
        else if (line.rfind("mkdir ", 0) == 0) {
            std::string dname = line.substr(6);
            fs::create_directory(current / dname);
            std::cout << "Created directory: " << dname << "\n";
        }
        else if (line.rfind("cp ", 0) == 0) {
            std::stringstream ss(line.substr(3));
            std::string src, dst; ss >> src >> dst;
            fs::path srcp = current / src, dstp = current / dst;
            try {
                fs::copy_file(srcp, dstp, fs::copy_options::overwrite_existing);
                std::cout << "Copied " << src << " -> " << dst << "\n";
            } catch (std::exception& e) {
                std::cout << "Copy failed: " << e.what() << "\n";
            }
        }
        else if (line.rfind("mv ", 0) == 0) {
            std::stringstream ss(line.substr(3));
            std::string src, dst; ss >> src >> dst;
            fs::path srcp = current / src, dstp = current / dst;
            try {
                fs::rename(srcp, dstp);
                std::cout << "Moved " << src << " -> " << dst << "\n";
            } catch (std::exception& e) {
                std::cout << "Move failed: " << e.what() << "\n";
            }
        }
        else if (line.rfind("rm ", 0) == 0) {
            std::string target = line.substr(3);
            fs::path targetp = current / target;
            try {
                if (fs::is_directory(targetp))
                    fs::remove_all(targetp);
                else
                    fs::remove(targetp);
                std::cout << "Removed: " << target << "\n";
            } catch (std::exception& e) {
                std::cout << "Remove failed: " << e.what() << "\n";
            }
        }
        // -------------------------------------

        else {
            std::cout << "Unknown command\n";
        }
    }

    return 0;
}




