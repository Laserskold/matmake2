#pragma once

#include "matmakefile.h"
#include "task.h"
#include "tasklist.h"
#include <memory>

std::vector<filesystem::path> expandPaths(std::string expression) {
    return {expression};
}

TaskList createTree(const MatmakeFile &file, const MatmakeNode &root) {
    TaskList taskList;

    auto in = root.property("in");

    if (in) {
        for (auto name : in->values) {
            if (name.empty()) {
                continue;
            }
            else if (name.front() == '@') {
                name = name.substr(1);
            }
            auto f = file.find(name);
            if (!f) {
                throw std::runtime_error{"could not find name " + name +
                                         " at " + std::string{in->pos}};
            }
            taskList.insert(createTree(file, *f));
        }
    }

    auto &task = taskList.emplace();

    task.name(root.name());
    if (auto p = root.property("dir")) {
        task.dir(p->value());
    }
    if (auto p = root.property("cxx")) {
        task.cxx(p->value());
    }
    if (auto p = root.property("command")) {
        task.command(p->value());
    }
    if (auto p = root.property("in")) {
    }

    return taskList;
}

TaskList createTasks(const MatmakeFile &file, std::string rootName) {
    for (auto &node : file.nodes()) {
        if (auto command = node.property("command")) {
            if (command->value() == "[root]") {
                if (node.name() == rootName) {
                    return createTree(file, node);
                }
            }
        }
    }

    return {};
}
