#include "explorer.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstdlib>

namespace fs = std::filesystem;

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

void print_ls(const std::vector<Entry>& entries, const fs::path& base, std::ostream& out) {
    out << std::left << std::setw(11) << "PERMS"
        << std::setw(10) << "SIZE"
        << "NAME\n";
    out << "---------------------------------------\n";

    for (const auto& e : entries) {
        fs::path full = base / e.name;
        std::string perms = "---------";
        try { perms = format_permissions(fs::status(full).permissions()); } catch (...) {}
        std::string size = e.is_dir ? "<DIR>" : std::to_string(e.size);
        out << std::left << std::setw(11) << perms
            << std::setw(10) << size
            << e.name << '\n';
    }
}

void find_pattern(const fs::path& base, const std::string& pattern, std::ostream& out) {
    try {
        for (auto& p : fs::recursive_directory_iterator(base)) {
            if (p.path().filename().string().find(pattern) != std::string::npos) {
                out << p.path().string() << "\n";
            }
        }
    } catch (const std::exception& e) {
        out << "Find error: " << e.what() << "\n";
    }
}

bool change_permissions(const fs::path& path, const std::string& octal) {
#ifdef _WIN32
    std::cerr << "Permission changes not supported on Windows.\n";
    return false;
#else
    char* end;
    long mode = strtol(octal.c_str(), &end, 8);
    if (end == octal.c_str() || *end != '\0') {
        std::cerr << "Invalid permission mode.\n";
        return false;
    }
    return (::chmod(path.c_str(), static_cast<mode_t>(mode)) == 0);
#endif
}

enum RedirectType { NONE, OVERWRITE, APPEND };
struct ParsedCmd {
    std::string cmd;
    RedirectType redirect = NONE;
    std::string filename;
};

ParsedCmd parse_redirect(const std::string& line) {
    ParsedCmd res;
    size_t pos = line.find(">>");
    if (pos != std::string::npos) {
        res.cmd = line.substr(0, pos);
        res.filename = line.substr(pos + 2);
        res.redirect = APPEND;
    } else {
        pos = line.find(">");
        if (pos != std::string::npos) {
            res.cmd = line.substr(0, pos);
            res.filename = line.substr(pos + 1);
            res.redirect = OVERWRITE;
        } else {
            res.cmd = line;
            res.redirect = NONE;
        }
    }

    auto trim = [](std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    };
    trim(res.cmd);
    trim(res.filename);
    return res;
}

int main() {
    fs::path current = fs::current_path();
    std::string rawline;

    while (true) {
        std::cout << current.string() << " $ ";
        std::getline(std::cin, rawline);
        if (rawline.empty()) continue;

        ParsedCmd parsed = parse_redirect(rawline);
        std::ostringstream output;
        std::ostream* out = &std::cout;
        bool redirected = (parsed.redirect != NONE);
        if (redirected) out = &output;

        std::string line = parsed.cmd;

        if (line == "exit") break;
        else if (line == "pwd") (*out) << current.string() << "\n";
        else if (line == "ls") {
            auto entries = list_directory(current.string());
            print_ls(entries, current, *out);
        }
        else if (line.rfind("cd ", 0) == 0) {
            std::string dir = line.substr(3);
            fs::path newp = (dir == "..") ? current.parent_path() : current / dir;
            if (fs::exists(newp) && fs::is_directory(newp))
                current = fs::canonical(newp);
            else
                (*out) << "No such directory.\n";
        }
        else if (line.rfind("touch ", 0) == 0) {
            std::string f = line.substr(6);
            std::ofstream(current / f);
        }
        else if (line.rfind("mkdir ", 0) == 0) {
            std::string d = line.substr(6);
            fs::create_directory(current / d);
        }
        else if (line.rfind("cp ", 0) == 0) {
            std::stringstream ss(line.substr(3));
            std::string src, dst; ss >> src >> dst;
            try { fs::copy_file(current / src, current / dst, fs::copy_options::overwrite_existing); }
            catch (...) { (*out) << "Copy failed.\n"; }
        }
        else if (line.rfind("mv ", 0) == 0) {
            std::stringstream ss(line.substr(3));
            std::string src, dst; ss >> src >> dst;
            try { fs::rename(current / src, current / dst); }
            catch (...) { (*out) << "Move failed.\n"; }
        }
        else if (line.rfind("rm ", 0) == 0) {
            std::string t = line.substr(3);
            fs::path target = current / t;
            try {
                if (fs::is_directory(target)) fs::remove_all(target);
                else fs::remove(target);
            } catch (...) { (*out) << "Remove failed.\n"; }
        }
        else if (line.rfind("find ", 0) == 0) {
            std::string pat = line.substr(5);
            find_pattern(current, pat, *out);
        }
        else if (line.rfind("perm ", 0) == 0) {
            std::stringstream ss(line.substr(5));
            std::string fname, mode; ss >> fname >> mode;
            fs::path p = current / fname;
            if (change_permissions(p, mode))
                (*out) << "Permissions changed for " << fname << " to " << mode << "\n";
            else
                (*out) << "Failed to change permissions.\n";
        }
        else {
            (*out) << "Unknown command.\n";
        }

        if (redirected) {
            fs::path file = parsed.filename;
            std::ofstream ofs;
            if (parsed.redirect == OVERWRITE)
                ofs.open(current / file, std::ofstream::trunc);
            else
                ofs.open(current / file, std::ofstream::app);

            ofs << output.str();
            std::cout << "Output redirected to " << (current / file).string() << "\n";
        }
    }

    return 0;
}




