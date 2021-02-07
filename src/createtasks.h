#pragma once

#include "matmakefile.h"
#include "prescan.h"
#include "sourcetype.h"
#include "task.h"
#include "tasklist.h"
#include <memory>

namespace task {

std::vector<filesystem::path> expandPaths(filesystem::path expression) {
    auto filename = expression.filename().string();
    if (auto f = filename.find('*'); f != std::string::npos) {
        auto dir = expression.parent_path();
        auto beginning = filename.substr(0, f);
        auto ending = filename.substr(f + 1);

        auto ret = std::vector<filesystem::path>{};
        for (auto it : filesystem::directory_iterator{"." / dir}) {
            if (it.path() == "." || it.path() == "..") {
                continue;
            }
            auto path = filesystem::relative(it.path(),
                                             "./"); // Remove "./" in beginning

            auto fn = path.filename().string();

            if (!beginning.empty()) {
                if (fn.find(beginning) != 0) {
                    continue;
                }
            }
            if (!ending.empty()) {
                if (fn.find(ending) != fn.size() - ending.size()) {
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
TaskList createTaskFromPath(filesystem::path path, bool useModules = true) {
    auto ret = TaskList{};

    auto type = SourceType{};

    try {
        type = sourceTypeMap.at(path.extension());
    }
    catch (...) {
        throw std::runtime_error{"unknown file ending: " + path.string()};
    }

    if (!useModules) {
        auto &source = ret.emplace();

        source.out("." / path);

        auto &task = ret.emplace();

        task.pushIn(&source);

        task.out(path.string() + ".o");

        task.command("[cxx]");

        task.depfile(path.string() + ".d");
    }
    else {
        auto &source = ret.emplace();

        source.out("." / path);

        auto &expandedSource = ret.emplace();

        expandedSource.pushIn(&source);

        expandedSource.out(path.string() + ".eem");

        expandedSource.command("[none]");

        if (getType(path) == SourceType::ModuleSource) {

            auto &precompiledModule = ret.emplace();

            precompiledModule.pushIn(&expandedSource);

            auto precompiledPath = path;
            precompiledPath.replace_extension(".pcm");

            precompiledModule.out(precompiledPath);

            precompiledModule.depfile(precompiledPath.string() + ".pcm.d");

            precompiledModule.command("[pcm]");

            auto &task = ret.emplace();

            task.pushIn(&precompiledModule);

            task.out(precompiledPath.string() + ".o");

            task.command("[cxxm]");
        }
        else {
            // No precompilation step is needed for ordinary cpp-files
            auto &task = ret.emplace();

            task.pushIn(&expandedSource);

            task.out(path.string() + ".o");

            task.command("[cxx]");
        }
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
    if (auto p = root.property("flags")) {
        task.flags(p->concat());
    }
    {
        auto &commands = root.ocommands();

        if (!commands.empty()) {
            task.commands(commands);
        }
    }

    task.generateDepName();

    return {std::move(taskList), &task};
}

} // namespace task

TaskList createTasks(const MatmakeFile &file, std::string rootName) {
    for (auto &node : file.nodes()) {
        if (auto command = node.property("command")) {
            if (command->value() == "[root]") {
                if (node.name() == rootName) {
                    auto tasks = task::createTree(file, node).first;
                    prescan(tasks);
                    calculateState(tasks);
                    return tasks;
                }
            }
        }
    }

    return {};
}
