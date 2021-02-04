#pragma once

#include "matmakefile.h"
#include "task.h"
#include "tasklist.h"
#include <memory>

std::vector<filesystem::path> expandPaths(filesystem::path expression) {
    auto filename = expression.filename().string();
    if (auto f = filename.find('*'); f != std::string::npos) {
        auto dir = expression.parent_path();
        auto beginning = filename.substr(0, f);
        auto ending = filename.substr(f + 1);

        auto ret = std::vector<filesystem::path>{};
        //        for (auto it : filesystem::recursive_directory_iterator{"."})
        //        {
        for (auto it : filesystem::directory_iterator{"." / dir}) {
            if (it.path() == "." || it.path() == "..") {
                continue;
            }
            auto path = filesystem::relative(it.path(),
                                             "./"); // Remove "./" in beginning

            auto fn = path.filename().string();

            if (!beginning.empty()) {
                if (fn.find(beginning) == std::string::npos) {
                    continue;
                }
            }
            if (!ending.empty()) {
                if (fn.find(ending) == std::string::npos) {
                    continue;
                }
            }

            ret.push_back(path);
        }

        return ret;
    }

    return {expression};
}

//! The last item is the one that is expected to be linked to
TaskList createTaskFromPath(filesystem::path path) {
    TaskList ret;

    if (path.extension() == ".cpp") {
        auto &source = ret.emplace();

        source.out("." / path);

        auto &task = ret.emplace();

        task.pushIn(&source);

        task.out(path.string() + ".o");

        task.command("[cxx]");

        task.depfile(path.string() + ".d");
    }

    return ret;
}

std::pair<TaskList, Task *> createTree(const MatmakeFile &file,
                                       const MatmakeNode &root) {
    TaskList taskList;

    auto in = root.property("in");

    auto &task = taskList.emplace();

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
            auto tree = createTree(file, *f);
            //            for (auto &t : list) {
            //                task.pushIn(t.get());
            //            }
            task.pushIn(tree.second);
            taskList.insert(std::move(tree.first));
        }
    }

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
    if (auto p = root.property("src")) {
        for (auto &src : p->values) {
            auto paths = expandPaths(src);

            for (auto &path : paths) {
                auto list = createTaskFromPath(path);
                if (!list.empty()) {
                    task.pushIn(&list.back());
                    taskList.insert(std::move(list));
                }
            }
        }
    }
    if (auto p = root.property("out")) {
        task.out(p->value());
    }
    {
        auto &commands = root.ocommands();

        if (!commands.empty()) {
            task.commands(commands);
        }
    }

    return {std::move(taskList), &task};
}

TaskList createTasks(const MatmakeFile &file, std::string rootName) {
    for (auto &node : file.nodes()) {
        if (auto command = node.property("command")) {
            if (command->value() == "[root]") {
                if (node.name() == rootName) {
                    auto tasks = createTree(file, node).first;
                    calculateState(tasks);
                    return tasks;
                }
            }
        }
    }

    return {};
}
