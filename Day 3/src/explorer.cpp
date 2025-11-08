#include "explorer.hpp"
#include <filesystem>
#include <iostream>   

namespace fs = std::filesystem;

std::vector<Entry> list_directory(const std::string& path) {
    std::vector<Entry> entries;

    try {
        for (const auto& p : fs::directory_iterator(path)) {
            Entry e;
            e.name = p.path().filename().string();
            e.is_dir = fs::is_directory(p);
            e.size = e.is_dir ? 0 : fs::file_size(p);  
            entries.push_back(e);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << '\n';
    }

    return entries;
}

