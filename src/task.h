#pragma once

#include "filesystem.h"
#include <algorithm>
#include <string>
#include <vector>

struct Task {
    Task() = default;
    Task(const Task &) = delete;
    Task(Task &&) = default;
    Task &operator=(const Task &) = delete;
    Task &operator=(Task &&) = default;

    std::string name() const {
        if (_name.empty()) {
            return _src.string();
        }
        else {
            return _name;
        }
    }

    void name(std::string value) {
        _name = std::move(value);
    }

    std::filesystem::path src() const {
        return _src;
    }

    std::filesystem::path out() const {
        return _out;
    }

    void out(filesystem::path path) {
        _out = path;
    }

    void pushIn(Task *in) {
        if (!in) {
            return;
        }
        if (std::find(_in.begin(), _in.end(), in) == _in.end()) {
            _in.push_back(in);
        }
    }

    auto &in() {
        return _in;
    }

private:
    Task *_parent = nullptr;
    filesystem::path _src;
    filesystem::path _out;

    std::string _name; // If empty-same as out

    size_t waiting = 0;
    std::vector<Task *> _in;
    //    std::vector<Task *> triggers; // This might not be needed

    std::vector<Task *> subscribers;

    std::string command; // If empty use parents
};
