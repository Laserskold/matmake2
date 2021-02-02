#pragma once

#include "filesystem.h"
#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class ProcessedCommand {
public:
    ProcessedCommand(std::string command) {
        auto f = size_t{0};
        auto old = size_t{0};

        while ((f = command.find('{', old)) != std::string::npos) {
            segments.push_back({
                .value = command.substr(old, f - old),
                .isReference = false,
            });

            if ((old = command.find('}', f)) != std::string::npos) {
                segments.push_back({
                    .value = command.substr(f + 1, old - f - 1),
                    .isReference = true,
                });
                ++old;
            }
            else {
                break;
            }
        }
    }
    ProcessedCommand(const ProcessedCommand &) = default;
    ProcessedCommand(ProcessedCommand &&) = default;
    ProcessedCommand &operator=(const ProcessedCommand &) = default;
    ProcessedCommand &operator=(ProcessedCommand &&) = default;

    std::string expandCommand(const struct Task &task);

private:
    struct Segment {
        std::string value;
        bool isReference = false;
    };

    std::vector<Segment> segments;
};

struct Task {
    Task() = default;
    Task(const Task &) = delete;
    Task(Task &&) = default;
    Task &operator=(const Task &) = delete;
    Task &operator=(Task &&) = default;

    std::string name() const {
        if (_name.empty()) {
            return _out.string();
        }
        else {
            return _name;
        }
    }

    void name(std::string value) {
        _name = std::move(value);
    }

    filesystem::path src() const {
        return _src;
    }

    filesystem::path out() const {
        return dir() / _out;
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

    void dir(filesystem::path dir) {
        _dir = dir;
    }

    filesystem::path dir() const {
        if (_parent) {
            if (_dir.empty()) {
                return _parent->dir();
            }
            else {
                return _parent->dir() / _dir; // Fix double slashes somhow
            }
        }
        else {
            return _dir;
        }
    }

    std::string property(std::string name) const {
        if (name == "command") {
            return _command;
        }
        else if (name == "out") {
            return out().string();
        }
        else if (name == "dir") {
            return dir().string();
        }
        else if (name == "depfile") {
            return depfile().string();
        }
        return {};
    }

    void depfile(filesystem::path path) {
        _depfile = path;
    }

    filesystem::path depfile() const {
        return _depfile;
    }

    void command(std::string command) {
        _command = command;
    }

    std::string command() const {
        if (_command.empty()) {
            if (_parent) {
                return _parent->command();
            }
        }
        else {
            if (_command.front() == '[' && _command.back() == ']') {
                return commandAt(_command.substr(1, _command.size() - 2));
            }
            else {
                return _command;
            }
        }
        throw std::runtime_error{"no command specified for target " + name()};
    }

    void commands(std::map<std::string, std::string> commands) {
        _commands = std::move(commands);
    }

    const std::map<std::string, std::string> &commands() const {
        if (_commands.empty() && _parent) {
            return _parent->commands();
        }
        return _commands;
    }

    // Get a single command from the commands-list
    std::string commandAt(std::string name) const {
        if (auto f = _commands.find(name); f != _commands.end()) {
            return f->second;
        }
        else if (_parent) {
            return _parent->commandAt(name);
        }
        else {
            throw std::runtime_error{"could not find " + name + " on target " +
                                     this->name()};
        }
    }

private:
    Task *_parent = nullptr;
    filesystem::path _src;
    filesystem::path _out;
    filesystem::path _dir;
    filesystem::path _depfile;

    std::string _name; // If empty-same as out

    size_t waiting = 0;
    std::vector<Task *> _in; // Files that needs to be built before this file
    std::vector<filesystem::path>
        triggers; // Files that mark this task as dirty

    std::vector<Task *> subscribers;

    std::string _command;                         // If empty use parents
    std::map<std::string, std::string> _commands; // Parents build command
};

std::string ProcessedCommand::expandCommand(const Task &task) {
    std::ostringstream ss;

    for (auto s : segments) {
        if (s.isReference) {
            ss << task.property(s.value);
        }
        else {
            ss << s.value;
        }
    }

    return ss.str();
}
