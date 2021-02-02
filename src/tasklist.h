#pragma once
#include "filesystem.h"
#include "task.h"
#include <vector>

struct TaskList {
    TaskList() = default;
    TaskList(const TaskList &) = delete;
    TaskList &operator=(const TaskList &) = delete;
    TaskList(TaskList &&) = delete;
    TaskList &operator=(TaskList &&) = delete;

    std::vector<Task> _tasks;

    void reserve(size_t size) {
        _tasks.reserve(size);
    }

    Task &emplace() {
        _tasks.emplace_back();
        return _tasks.back();
    }

    Task *find(std::string name) {
        if (name.front() == '@') {
            name = name.substr(1);
            for (auto &t : _tasks) {
                if (t.name() == name) {
                    return &t;
                }
            }
        }
        else {
            for (auto &t : _tasks) {
                if (t.out() == name) {
                    return &t;
                }
            }
        }

        return nullptr;
    }

    Task &at(size_t i) {
        return _tasks.at(i);
    }

    auto begin() {
        return _tasks.begin();
    }

    auto end() {
        return _tasks.end();
    }

    auto begin() const {
        return _tasks.begin();
    }

    auto end() const {
        return _tasks.end();
    }
};
