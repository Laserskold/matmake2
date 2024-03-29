﻿// copyright Mattias Larsson Sköld 2021

#include "coordinator.h"
#include "createtasks.h"
#include "filesystem.h"
#include "makefile.h"
#include "matmakefile.h"
#include "msvcenvironment.h"
#include "ninja.h"
#include "parsematmakefile.h"
#include "settings.h"
#include "tasklist.h"
#include "test.h"
#include "json/json.h"

namespace {

TaskList createTasksFromMatmakefile(const Settings &settings) {
    auto getJson = [&]() -> Json {
        if (filesystem::exists("Matmakefile")) {
            return parseMatmakefile("Matmakefile");
        }
        else if (filesystem::exists("matmake.json")) {
            return Json::LoadFile("matmake.json");
        }
        else {
            std::cerr << "no matmakefile found in directory\n";
            return {};
        }
    };

    auto matmakeFile = MatmakeFile{getJson(), settings.target};

    if (settings.debugPrint) {
        matmakeFile.print(std::cout);
    }

    return createTasks(matmakeFile, settings.target);
}

int parseTasksCommand(const Settings settings) {
    if (!settings.taskFile.empty()) {
        filesystem::current_path(settings.taskFile.parent_path());

        auto tasks = parseTasks(settings.taskFile.filename());

        if (settings.printTasks) {
            printFlat(*tasks);
        }

        if (settings.printTree) {
            std::cout << "\n"
                         "treeview\n"
                         "==================\n";
            auto &root = *tasks->find("@" + settings.target);
            root.print(settings.verbose);
        }

        std::cout << "building... \n";

        if (!settings.skipBuild) {
            auto coordinator = Coordinator{};

            auto status = coordinator.execute(*tasks, settings);

            if (status) {
                std::cout << "failed...\n";
            }
            else {
                std::cout << "done...\n";
            }

            return status;
        }
    }

    std::cerr << "could not open task file\n";

    return 1;
}

void outputCompileCommands(const TaskList &tasks) {
    auto json = Json{Json::Array};

    auto dir = filesystem::absolute(filesystem::current_path());
    for (auto &task : tasks) {
        if (getType(task->out()) != SourceType::Object) {
            continue;
        }
        auto command = task->command();
        auto taskJson = Json{Json::Object};

        taskJson["directory"] = dir.string();
        taskJson["command"] = ProcessedCommand{command}.expand(*task);
        auto source = task->findSource();
        if (!source) {
            std::cerr << "could not find source file for " << task->out();
            continue;
        }
        auto file = source->out().string();
        if (file.rfind("./", 0) == 0) {
            file = file.substr(2);
        }
        taskJson["file"] = file;

        json.push_back(std::move(taskJson));
    }

    std::ofstream{"compile_commands.json"} << json;
}

int build(const Settings &settings) {
    auto tasks = createTasksFromMatmakefile(settings);

    if (settings.target.empty()) {
        throw std::runtime_error{
            "no target specified. Use \"--target\" to specify"};
    }

    if (tasks.empty()) {
        throw std::runtime_error{"could not find target " + settings.target};
    }

    if (settings.printTree) {
        std::cout << "\n"
                     "treeview\n"
                     "==================\n";
        auto root = tasks.find("@" + settings.target);

        if (!root) {
            throw std::runtime_error{"could not find target " +
                                     settings.target};
        }

        root->print(settings.verbose);
        std::cout.flush();
    }

    if (settings.outputCompileCommands) {
        outputCompileCommands(tasks);
    }

    if (!settings.skipBuild) {
        auto coordinator = Coordinator{};
        auto status = coordinator.execute(tasks, settings);

        if (status) {
            std::cout << "failed...\n";
        }
        else {
            std::cout << "done...\n";
        }

        if (settings.command != Command::BuildAndTest || status) {
            return status;
        }
    }

    if (settings.command == Command::BuildAndTest) {
        return test(tasks, settings);
    }

    return 0;
}

int list(const Settings &settings) {
    auto json = [] {
        if (filesystem::exists("Matmakefile")) {
            return parseMatmakefile("Matmakefile");
        }

        return Json::LoadFile("matmake.json");
    }();

    auto matmakeFile = MatmakeFile{json};

    for (auto &node : matmakeFile.nodes()) {
        if (node.isRoot()) {
            std::cout << node.name() << "\n";
        }
    }

    return 0;
}

int clean(const Settings settings) {
    auto tasks = createTasksFromMatmakefile(settings);

    for (auto &task : tasks) {
        if (task->clean() && settings.verbose) {
            std::cout << "removed " << task->out() << "\n";
        }
    }

    return 0;
}

} // namespace

int main(int argc, char **argv) {
    const auto settings = Settings{argc, argv};

    try {
        if (settings.useMsvcEnvironment) {
            setMsvcEnvironment();
        }

        switch (settings.command) {
        case Command::ParseTasks:
            return parseTasksCommand(settings);
            break;
        case Command::Build:
        case Command::BuildAndTest: {
            switch (settings.backend) {
            case Backend::Default:
            case Backend::Ninja:
                return printNinja(settings,
                                  createTasksFromMatmakefile(settings));
                break;
            case Backend::Makefile:
                return printMakefile(settings,
                                     createTasksFromMatmakefile(settings));
                break;
            case Backend::Native:
                return build(settings);
            }
        } break;
        case Command::List: {
            return list(settings);
        } break;
        case Command::Clean: {
            return clean(settings);
        } break;
        }
    }
    catch (std::runtime_error &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
