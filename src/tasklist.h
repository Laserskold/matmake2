#pragma once
#include "filesystem.h"
#include "task.h"
#include <memory>
#include <vector>

struct TaskList {
    TaskList() = default;
    TaskList(const TaskList &) = delete;
    TaskList &operator=(const TaskList &) = delete;
    TaskList(TaskList &&) = delete;
    TaskList &operator=(TaskList &&) = delete;

    std::vector<std::unique_ptr<Task>> _tasks;

    void reserve(size_t size) {
        _tasks.reserve(size);
    }

    Task &emplace() {
        _tasks.emplace_back(std::make_unique<Task>());
        return *_tasks.back();
    }

    Task *find(std::string name) {
        if (name.empty()) {
            return nullptr;
        }
        if (name.front() == '@') {
            name = name.substr(1);
            for (auto &t : _tasks) {
                if (t->name() == name) {
                    return t.get();
                }
            }
        }
        else if (name.rfind("./") == 0) {
            for (auto &t : _tasks) {
                if (t->rawOut() == name) {
                    return t.get();
                }
            }
        }
        else {
            for (auto &t : _tasks) {
                if (t->out() == name) {
                    return t.get();
                }
            }
        }

        return nullptr;
    }

    Task &at(size_t i) {
        return *_tasks.at(i);
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

std::unique_ptr<TaskList> parseTasks(filesystem::path path);

void printFlat(const TaskList &list);
