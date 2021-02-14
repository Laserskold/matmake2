// copyright Mattias Larsson Sköld 2021

#include "coordinator.h"
#include "createtasks.h"
#include "filesystem.h"
#include "matmakefile.h"
#include "parsematmakefile.h"
#include "settings.h"
#include "tasklist.h"
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

    auto matmakeFile = MatmakeFile{getJson()};

    if (settings.debugPrint) {
        matmakeFile.print(std::cout);
    }

    return createTasks(matmakeFile, settings.target);
};

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
            auto &root = *tasks->find("@g++");
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

int execute(std::string filename, filesystem::path path) {
    auto originalPath = filesystem::absolute(filesystem::current_path());
    filesystem::current_path(path);
    std::cout << "running " << filename << " in " << path << std::endl;

    if constexpr (getOs() == Os::Linux) {
        filename = "./" + filename;
    }

    auto res = std::system(filename.c_str());

    filesystem::current_path(originalPath);

    return res;
}

int test(const TaskList &tasks, const Settings &settings) {
    std::vector<const Task *> tests;

    for (auto &task : tasks) {
        if (task->isTest()) {
            auto exe = task->out();
            auto path = exe.parent_path();
            auto command = exe.filename();
            if (execute(command, path)) {
                std::cout << "failed...";
            }
            else {
                std::cout << "succsess...";
            }
        }
    }

    return 0;
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
        if (auto p = node.property("command"); p && p->concat() == "[root]") {
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

    switch (settings.command) {
    case Command::ParseTasks:
        return parseTasksCommand(settings);
        break;
    case Command::Build:
    case Command::BuildAndTest: {
        return build(settings);
    } break;
    case Command::List: {
        return list(settings);
    } break;
    case Command::Clean: {
        return clean(settings);
    } break;
    }

    return 0;
}
