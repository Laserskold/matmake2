#pragma once

#include "matmakefile.h"
#include "task.h"
#include <memory>

std::vector<std::unique_ptr<Task>> createTree(const MatmakeFile &file,
                                              const MatmakeNode &root) {
    std::vector<std::unique_ptr<Task>> ret;

    auto in = root.property("in");

    if (in) {
        for (auto name : in->values) {
            if (name.empty()) {
                continue;
            }
            else if (name.front() == '@') {
                name = name.substr(1);
            }
            //            std::cout << name << "\n";
            auto f = file.find(name);
            if (!f) {
                throw std::runtime_error{"could not find name " + name +
                                         " at " + std::string{in->pos}};
            }
            auto list = createTree(file, *f);

            for (auto &it : list) {
                ret.push_back(std::move(it));
            }
        }
    }

    auto &task = ret.emplace_back(std::make_unique<Task>());

    task->name(root.name());
    if (auto p = root.property("dir")) {
        task->dir(p->value());
    }
    if (auto p = root.property("cxx")) {
        task->cxx(p->value());
    }
    if (auto p = root.property("command")) {
        task->command(p->value());
    }
    return ret;
}

std::vector<std::unique_ptr<Task>> createTasks(const MatmakeFile &file,
                                               std::string rootName) {
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
