// copyright Mattias Larsson Sköld 2021

#include "filesystem.h"
#include "task.h"
#include "tasklist.h"
#include "json/json.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

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
