#include "explorer.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <thread>
#include <chrono>
#include <sys/stat.h>   // chmod()
#include <pwd.h>        // getpwuid()
#include <grp.h>        // getgrgid()
#include <cstring>      // strerror()
#include <cerrno>       // errno

namespace fs = std::filesystem;

// ===== Helper: Format Permissions =====
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

// ===== Helper: Show file info with owner/group =====
void show_permissions(const fs::path& file) {
    try {
        if (!fs::exists(file)) {
            std::cout << "No such file or directory: " << file.string() << "\n";
            return;
        }
        fs::perms p = fs::status(file).permissions();
        std::string perm_str = format_permissions(p);

#ifndef _WIN32
        struct stat sb;
        if (stat(file.c_str(), &sb) == 0) {
            struct passwd *pw = getpwuid(sb.st_uid);
            struct group  *gr = getgrgid(sb.st_gid);
            std::cout << perm_str << "  "
                      << (pw ? pw->pw_name : "?") << ":"
                      << (gr ? gr->gr_name : "?") << "  "
                      << (fs::is_directory(file) ? "<DIR>" : std::to_string(fs::file_size(file)))
                      << "  " << file.filename().string() << "\n";
        } else {
            std::cout << perm_str << "  " << file.filename().string() << "\n";
        }
#else
        std::cout << perm_str << "  " << file.filename().string() << "\n";
#endif
    } catch (const std::exception& e) {
        std::cerr << "Error reading permissions: " << e.what() << "\n";
    }
}

// ===== Helper: Change file permissions =====
bool change_permissions(const fs::path& path, const std::string& octal) {
#ifndef _WIN32
    char* end;
    long mode = strtol(octal.c_str(), &end, 8);
    if (end == octal.c_str() || *end != '\0') {
        std::cerr << "Invalid octal mode. Use like 644 or 755.\n";
        return false;
    }
    if (::chmod(path.c_str(), static_cast<mode_t>(mode)) != 0) {
        std::cerr << "chmod failed: " << strerror(errno) << "\n";
        return false;
    }
    return true;
#else
    std::cerr << "Permission changes not supported on Windows.\n";
    return false;
#endif
}

// ===== Core Command Loop =====
int main() {
    fs::path current = fs::current_path();
    std::string line;

    while (true) {
        std::cout << current.string() << " $ ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        if (line == "exit") break;

        // ===== Basic Commands =====
        else if (line == "pwd") std::cout << current.string() << "\n";

        else if (line == "ls") {
            auto entries = list_directory(current.string());
            std::cout << std::left << std::setw(11) << "PERMS"
                      << std::setw(10) << "SIZE" << "NAME\n";
            std::cout << "---------------------------------------\n";
            for (auto& e : entries) {
                fs::path f = current / e.name;
                std::string perms = format_permissions(fs::status(f).permissions());
                std::string size = e.is_dir ? "<DIR>" : std::to_string(e.size);
                std::cout << std::left << std::setw(11) << perms
                          << std::setw(10) << size << e.name << "\n";
            }
        }

        else if (line.rfind("cd ", 0) == 0) {
            std::string dir = line.substr(3);
            fs::path newp = (dir == "..") ? current.parent_path() : current / dir;
            if (fs::exists(newp) && fs::is_directory(newp)) current = fs::canonical(newp);
            else std::cout << "No such directory.\n";
        }

        // ===== Permission Commands =====
        else if (line.rfind("perms ", 0) == 0) {
            std::string target = line.substr(6);
            fs::path f = current / target;
            show_permissions(f);
        }

        else if (line.rfind("perm ", 0) == 0) {
            std::stringstream ss(line.substr(5));
            std::string file, mode;
            ss >> file >> mode;
            if (file.empty() || mode.empty()) {
                std::cout << "Usage: perm <file> <octal_mode>\n";
                continue;
            }
            fs::path f = current / file;
            if (!fs::exists(f)) {
                std::cout << "File not found: " << file << "\n";
                continue;
            }
            if (change_permissions(f, mode)) {
                std::cout << "Permissions changed for " << file << " to " << mode << "\n";
            }
        }

        else if (line == "help") {
            std::cout << "Available commands:\n"
                      << "  ls               - List files\n"
                      << "  cd <dir>         - Change directory\n"
                      << "  pwd              - Print working directory\n"
                      << "  perms <file>     - View file permissions\n"
                      << "  perm <f> <octal> - Change file permissions\n"
                      << "  exit             - Exit program\n";
        }

        else std::cout << "Unknown command. Type 'help' for a list.\n";
    }

    return 0;
}







