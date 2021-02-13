#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

inline std::string strip(std::string str) {
    while (!str.empty() && isspace(str.back())) {
        str.pop_back();
    }

    auto f = std::find_if(
        str.begin(), str.end(), [](auto x) { return !isspace(x); });

    if (f != str.end()) {
        return str.substr(std::distance(str.begin(), f));
    }

    return str;
}

struct Line {
    Line(std::istream &stream) {
        std::string line;

        while (stream) {
            getline(stream, line);

            if (auto f = line.find('#'); f != std::string::npos) {
                line.erase(line.begin() + f, line.end());
            }

            if (!line.empty()) {
                break;
            }
        }

        assign(line);
    }

    Line(std::string line) {

        assign(line);
    }

    void assign(std::string line) {
        if (line.empty()) {
            type = End;
            return;
        }

        for (auto c : line) {
            if (isspace(c)) {
                ++indent;
            }
            else {
                break;
            }
        }

        if (auto f = line.find('='); f != std::string::npos) {
            name = strip(line.substr(0, f));
            auto val = strip(line.substr(f + 1));

            if (val.empty()) {
                type = UnfinishedAssignment;
            }
            else {
                value = {std::move(val)};
                type = Assignment;
            }
        }
        else {
            type = Normal;
            name = strip(line);
        }
    }

    Line() = default;

    std::string name;
    std::vector<std::string> value;
    size_t indent = 0;

    enum {
        End,
        Normal,
        Assignment,
        UnfinishedAssignment,
    } type = End;
};
