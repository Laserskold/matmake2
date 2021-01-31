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
            in->parent(this);
        }
    }

    auto &in() {
        return _in;
    }

    auto &in() const {
        return _in;
    }

    void parent(Task *parent) {
        _parent = parent;
    }

    auto parent() {
        return _parent;
    }

    auto parent() const {
        return _parent;
    }

private:
    Task *_parent = nullptr;
    filesystem::path _src;
    filesystem::path _out;

    std::string _name; // If empty-same as out

    size_t waiting = 0;
    std::vector<Task *> _in; // Files that needs to be built before this file
    std::vector<filesystem::path>
        triggers; // Files that mark this task as dirty

    std::vector<Task *> subscribers;

    std::string command; // If empty use parents
};
