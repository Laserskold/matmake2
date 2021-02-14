#pragma once

#include "filesystem.h"
#include <fstream>
#include <sstream>
#include <vector>

struct DepFileContent {
    std::vector<filesystem::path> deps;
};

DepFileContent parseDepFile(filesystem::path path) {
    DepFileContent ret;
    auto file = std::ifstream{path};

    if (!file.is_open()) {
        return {};
    }

    bool isFirstLine = true;

    for (std::string line; std::getline(file, line);) {
        std::string depfile;
        std::istringstream ss(line);
        if (isFirstLine) {
            ss >> depfile;
            isFirstLine = false;
        }
        bool foundBackslash = false;
        for (std::string word; ss >> word;) {
            if (word.back() == '\\') {
                foundBackslash = true;
                word.pop_back();
            }

            else if (!word.empty()) {
                if (word.front() != '/') {
                    ret.deps.push_back("./" + word);
                }
            }
        }
        if (!foundBackslash) {
            break;
        }
    }
    return ret;
}
