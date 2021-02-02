// copyright Mattias Larsson Sköld 2021

#include "filesystem.h"
#include "task.h"
#include "taskio.h"
#include "tasklist.h"
#include "json/json.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

void connectTasks(TaskList &list, const Json &json) {
    for (size_t i = 0; i < json.size(); ++i) {
        auto &task = list.at(i);
        auto &data = json.at(i);

        if (auto f = data.find("in"); f != data.end()) {
            if (f->type == Json::Array) {
                for (auto &value : *f) {
                    task.pushIn(list.find(value.string()));
                }
            }
            else {
                task.pushIn(list.find(f->value));
            }
        }
    }
}

std::unique_ptr<TaskList> parseTasks(filesystem::path path) {
    auto list = std::make_unique<TaskList>();
    auto json = Json{};
    auto file = std::ifstream{path};

    if (!file.is_open()) {
        throw std::runtime_error{"could not load tasks from " + path.string()};
    }
    file >> json;

    if (json.type != Json::Array) {
        throw std::runtime_error{"expected array in " + path.string()};
    }

    list->reserve(json.size());

    for (const auto &jtask : json) {
        list->emplace().parse(jtask);
    }

    connectTasks(*list, json);

    return list;
}

void printFlat(const TaskList &list) {
    for (auto &task : list) {
        std::cout << "task: name = " << task.name() << "\n";
        std::cout << "  out = " << task.out() << "\n";
        if (task.parent()) {
            std::cout << "  parent = " << task.parent()->out() << "\n";
        }

        {
            auto &in = task.in();
            for (auto &i : in) {
                std::cout << "  in: " << i->out() << "\n";
            }
        }
    }
}

int main(int, char **) {
    std::cout << "hello\n" << std::endl;

    filesystem::current_path("demos/project1");
    auto tasks = parseTasks("tasks.json");

    printFlat(*tasks);

    std::cout << "\ntreeview\n==================\n";
    auto &root = *tasks->find("@g++");
    root.print();

    std::cout << "root [pch] = " << root.commandAt("pch") << "\n";

    return 0;
}
