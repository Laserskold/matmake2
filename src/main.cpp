// copyright Mattias Larsson Sköld 2021

#include "filesystem.h"
#include "task.h"
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

void parseTask(Task &task, const Json &jtask) {
    auto jsonFind = [&jtask](std::string name) -> const Json * {
        auto f = jtask.find(name);
        if (f != jtask.end()) {
            return &*f;
        }
        else {
            return nullptr;
        }
    };
    //    auto &task = list->emplace();
    if (auto f = jsonFind("name")) {
        task.name(f->string());
    }
    if (auto f = jsonFind("out")) {
        task.out(f->string());
    }
    if (auto f = jsonFind("command")) {
        task.command(f->string());
    }
    if (auto f = jsonFind("commands")) {
        if (f->type == Json::Object) {
            std::map<std::string, std::string> commands;
            for (auto &child : *f) {
                commands[child.name] = child.value;
            }

            task.commands(std::move(commands));
        }
    }
    if (auto f = jsonFind("dir")) {
        task.dir(f->string());
    }
    if (auto f = jsonFind("depfile")) {
        task.depfile(f->string());
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
        parseTask(list->emplace(), jtask);
    }

    connectTasks(*list, json);

    return list;
}

void printList(const TaskList &list) {
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

void printTree(const Task &root, size_t indentation = 0) {
    auto indent = [indentation]() -> std::ostream & {
        for (size_t i = 0; i < indentation; ++i) {
            std::cout << "  ";
        }
        return std::cout;
    };

    indent();
    std::cout << root.name() << " " << root.out() << "\n";

    try {
        {
            auto command = root.command();
            indent();
            std::cout << "command: "
                      << ProcessedCommand{command}.expandCommand(root) << "\n";

            indent();
            std::cout << "raw: " << command << "\n";
        }
    }
    catch (...) {
    }

    indent() << "dir: " << root.dir() << "\n";

    {
        auto commands = root.commands();
        if (!commands.empty()) {
            indent() << "defined commands\n";
        }

        for (const auto &command : commands) {
            indent() << "  \"" << command.first << " = " << command.second
                     << "\"\n";
        }
    }

    if (!root.in().empty()) {
        indent() << "in:\n";
    }
    for (auto &in : root.in()) {
        printTree(*in, indentation + 1);
    }
}

int main(int, char **) {
    std::cout << "hello\n" << std::endl;

    filesystem::current_path("demos/project1");
    auto tasks = parseTasks("tasks.json");

    printList(*tasks);

    std::cout << "\ntreeview\n==================\n";
    auto &root = *tasks->find("@g++");
    printTree(root);

    std::cout << "root [pch] = " << root.commandAt("pch") << "\n";

    return 0;
}
