#pragma once

#include "matmakefile.h"
#include "prescan.h"
#include "sourcetype.h"
#include "task.h"
#include "tasklist.h"
#include "translateconfig.h"
#include <memory>

namespace task {

inline std::vector<filesystem::path> expandPaths(filesystem::path expression) {
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
            if (filesystem::is_directory(it.path())) {
                continue;
            }
#ifdef __cpp_lib_experimental_filesystem
            auto path = filesystem::path{it.path().string().substr(2)};
#else
            auto path = filesystem::relative(it.path(),
                                             "./"); // Remove "./" in beginning
#endif

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
inline TaskList createTaskFromPath(filesystem::path path,
                                   FlagStyle style,
                                   bool useModules = true) {
    auto ret = TaskList{};

    auto type = SourceType{};

    if (path.empty()) {
        return {};
    }

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

        task.out(path.string() + extension(".o", style));

        task.command("[cxx]");

        task.buildLocation(BuildLocation::Intermediate);

        task.depfile(path.string() + ".d");
    }
    else {
        auto &source = ret.emplace();

        source.out("." / path);

        auto &expandedSource = ret.emplace();

        expandedSource.pushIn(&source);

        expandedSource.out(path.string() + ".eem");

        expandedSource.command("[none]");

        expandedSource.buildLocation(BuildLocation::Intermediate);

        if (type == SourceType::ModuleSource) {

            auto &precompiledModule = ret.emplace();

            precompiledModule.pushIn(&expandedSource);

            auto precompiledPath = path;
            precompiledPath.replace_extension(".pcm");

            precompiledModule.out(precompiledPath);

            precompiledModule.depfile(precompiledPath.string() + ".pcm.d");

            precompiledModule.command("[pcm]");

            precompiledModule.buildLocation(BuildLocation::Intermediate);

            auto &task = ret.emplace();

            task.pushIn(&precompiledModule);

            task.out(precompiledPath.string() + extension(".o", style));

            task.command("[cxxm]");

            task.buildLocation(BuildLocation::Intermediate);
        }
        else {
            // No precompilation step is needed for ordinary cpp-files
            auto &task = ret.emplace();

            task.pushIn(&expandedSource);

            task.out(path.string() + extension(".o", style));

            task.command((type == SourceType::CSource) ? "[cc]" : "[cxx]");

            task.buildLocation(BuildLocation::Intermediate);
        }
    }

    return ret;
}

//! Create a task to copy a single file
inline TaskList createCopyTaskFromPath(std::string pattern) {
    TaskList ret;

    auto createCopyTask = [&ret](filesystem::path path) {
        auto &source = ret.emplace();

        source.out("." / path);
        source.command("[none]");

        auto &task = ret.emplace();
        task.pushIn(&source);
        task.out(path);

        task.command("[copy]");
    };

    if (filesystem::exists(pattern)) {
        if (filesystem::is_directory(pattern)) {
            for (auto &it : filesystem::recursive_directory_iterator{pattern}) {
                if (!filesystem::is_directory(it.path())) {
                    createCopyTask(it.path());
                }
            }
        }
        else {
            createCopyTask(pattern);
        }
    }
    else {
        for (auto path : expandPaths(pattern)) {
            createCopyTask(pattern);
        }
    }

    return ret;
}

inline std::pair<TaskList, Task *> createTree(
    const MatmakeFile &file,
    const MatmakeNode &root,
    std::map<filesystem::path, Task *> &duplicateMap,
    FlagStyle style) {
    TaskList taskList;

    auto in = root.property("in");

    auto &task = taskList.emplace();

    task.name(root.name());

    if (auto p = root.property("flagstyle")) {
        // Needs to be first
        task.flagStyle(p->value());

        style = task.flagStyle();
    }
    if (auto p = root.property("dir")) {
        task.dir(BuildLocation::Real, p->value());
    }
    if (auto p = root.property("objdir")) {
        task.dir(BuildLocation::Intermediate, p->value());
    }
    if (auto p = root.property("cxx")) {
        task.cxx(p->value());
    }
    if (auto p = root.property("cc")) {
        task.cc(p->value());
    }
    if (auto p = root.property("ar")) {
        task.ar(p->value());
    }
    if (auto p = root.property("command")) {
        task.command(p->value());
    }
    if (auto p = root.property("src")) {
        // Requires command to be red before becauso of flag style
        for (auto &src : p->values) {
            auto paths = expandPaths(src);

            for (auto &path : paths) {
                if (auto f = duplicateMap.find(path); f != duplicateMap.end()) {
                    task.pushIn(f->second);
                }
                else {
                    auto list = createTaskFromPath(path, style);
                    if (!list.empty()) {
                        task.pushIn(&list.back());
                        duplicateMap[path] = &list.back();
                        taskList.insert(std::move(list));
                    }
                }
            }
        }
    }
    if (auto p = root.property("copy")) {
        for (auto &c : p->values) {
            auto list = createCopyTaskFromPath(c);
            for (auto &copyTask : list) {
                if (copyTask->command() == "copy") {
                    task.pushIn(copyTask.get());
                }
            }
            taskList.insert(std::move(list));
        }
    }
    if (in) {
        // Its important that targets from in is included after src
        // otherwise there could be problems with linking to for example .a
        // files
        for (auto name : in->values) {
            if (name.empty()) {
                continue;
            }
            else if (name.front() == '@') {
                name = name.substr(1);
            }
            auto f = file.find(name);
            if (!f) {
                throw std::runtime_error{"could not find name '" + name +
                                         "' at " + std::string{in->pos}};
            }
            auto tree = createTree(file, *f, duplicateMap, style);
            task.pushIn(tree.second);
            taskList.insert(std::move(tree.first));
        }
    }
    if (auto p = root.property("out")) {
        task.out(p->value());
    }
    if (auto p = root.property("flags")) {
        task.flags(p->concat());
    }
    if (auto p = root.property("ldflags")) {
        task.ldflags(p->concat());
    }
    if (auto p = root.property("eflags")) {
        task.eflags(p->concat());
    }
    if (auto p = root.property("includes")) {
        for (auto &include : p->values) {
            task.pushInclude(include);
        }
    }
    if (auto p = root.property(("depprefix"))) {
        task.depfile(p->value());
    }
    if (auto p = root.property("config")) {
        task.config(p->values);
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

inline TaskList createTasks(const MatmakeFile &file, std::string rootName) {
    for (auto &node : file.nodes()) {
        if (auto command = node.property("command")) {
            if (command->value() == "[root]") {
                if (node.name() == rootName) {
                    // This map keeps track of o-files so that there is not
                    // multiple versions of the same file
                    auto duplicateMap = std::map<filesystem::path, Task *>{};
                    auto tasks =
                        task::createTree(
                            file, node, duplicateMap, FlagStyle::Inherit)
                            .first;
                    prescan(tasks);
                    calculateState(tasks);
                    return tasks;
                }
            }
        }
    }

    return {};
}
