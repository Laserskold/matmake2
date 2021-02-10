// copyright Mattias Larsson Sköld 2021

#include "coordinator.h"
#include "createtasks.h"
#include "filesystem.h"
#include "matmakefile.h"
#include "settings.h"
#include "tasklist.h"
#include "json/json.h"

auto createTasksFromMatmakefile = [](const Settings &settings) -> TaskList {
    if (!filesystem::exists(settings.matmakeFile)) {
        std::cerr << "no matmakefile found in directory\n";
        return {};
    }

    auto json = Json::loadFile(settings.matmakeFile.string());

    auto matmakeFile = MatmakeFile{json};

    if (settings.debugPrint) {
        matmakeFile.print(std::cout);
    }

    return createTasks(matmakeFile, settings.target);
};

int main(int argc, char **argv) {
    const auto settings = Settings{argc, argv};

    switch (settings.command) {
    case Command::ParseTasks:
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
        break;
    case Command::Build: {
        auto tasks = createTasksFromMatmakefile(settings);

        if (tasks.empty()) {
            throw std::runtime_error{"could not find target " +
                                     settings.target};
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

            return status;
        }
    } break;
    case Command::List: {
        auto json = Json::loadFile(settings.matmakeFile.string());

        auto matmakeFile = MatmakeFile{json};

        for (auto &node : matmakeFile.nodes()) {
            if (auto p = node.property("command");
                p && p->concat() == "[root]") {
                std::cout << node.name() << "\n";
            }
        }

    } break;
    case Command::Clean: {
        auto tasks = createTasksFromMatmakefile(settings);

        for (auto &task : tasks) {
            if (task->clean() && settings.verbose) {
                std::cout << "removed " << task->out() << "\n";
            }
        }

    } break;
    }

    return 0;
}
