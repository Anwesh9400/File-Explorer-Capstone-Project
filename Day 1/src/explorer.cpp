#include "explorer.h"
#include <filesystem>
#include <iostream>
#include <iomanip>

namespace fs = std::filesystem;

namespace explorer {
    void list_directory(const std::string& path) {
        try {
            for (auto &p : fs::directory_iterator(path)) {
                auto f = p.path();
                auto sz = fs::is_regular_file(f) ? fs::file_size(f) : 0;
                auto perms = p.status().permissions();
                std::cout << (fs::is_directory(f) ? "d " : "- ")
                          << std::setw(10) << sz << " "
                          << f.filename().string() << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}
