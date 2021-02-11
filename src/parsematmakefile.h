
#pragma once
#include "matmakefile.h"

#include "json/json.h"
#include <algorithm>
#include <iostream>

std::string strip(std::string str) {
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
            type = Assignment;

            name = strip(line.substr(0, f));
            value = {strip(line.substr(f + 1))};
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
    } type = End;
};

auto parseMatmakefile(std::istream &file) {

    auto json = Json{Json::Array};

    auto lastTarget = Json{};

    for (auto line = Line{file}; line.type != Line::End; line = Line{file}) {
        switch (line.type) {
        case Line::Normal:
            if (!lastTarget.empty()) {
                json.push_back(lastTarget);
            }
            lastTarget = Json{Json::Object};
            break;
        case Line::Assignment:
            if (line.value.size() == 1) {
                lastTarget[line.name] = line.value.front();
            }
            else if (line.value.size() > 1) {
                lastTarget[line.name].assign(line.value.begin(),
                                             line.value.end());
            }
            break;
        default:
            break;
        }
    }

    if (!lastTarget.empty()) {
        json.push_back(lastTarget);
    }

    return json;
}

Json parseMatmakefile(filesystem::path path) {
    auto file = std::ifstream{path};

    if (!file.is_open()) {
        throw std::runtime_error{"could not open matmakefile: " +
                                 path.string()};
    }

    return parseMatmakefile(file);
}
