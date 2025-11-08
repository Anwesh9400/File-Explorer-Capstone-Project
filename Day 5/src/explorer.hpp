#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct Entry {
    std::string name;
    bool is_dir;
    std::uintmax_t size;
};

std::vector<Entry> list_directory(const std::string& path);
