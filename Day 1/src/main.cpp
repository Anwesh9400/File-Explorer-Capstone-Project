#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
int main() {
    std::cout << "File Explorer Started\n";
    for (const auto &entry : fs::directory_iterator(fs::current_path()))
        std::cout << entry.path().filename() << '\n';
    return 0;
}
