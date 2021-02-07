// copyright Mattias Larsson Sköld 2021

#include "coordinator.h"
#include "createtasks.h"
#include "filesystem.h"
#include "matmakefile.h"
#include "settings.h"
#include "tasklist.h"
#include "json/json.h"

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
        if (!filesystem::exists(settings.matmakeFile)) {
            std::cerr << "no matmakefile found in directory\n";
            return 1;
        }

        auto json = Json::loadFile(settings.matmakeFile.string());

        auto matmakeFile = MatmakeFile{json};

        matmakeFile.print(std::cout);

        auto tasks = createTasks(matmakeFile, settings.target);

        for (auto &t : tasks) {
            //            std::cout << t->name() << std::endl;
            std::cout << t->dump() << std::endl;
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
    case Command::Clean: {
        auto json = Json::loadFile(settings.matmakeFile.string());

        auto matmakeFile = MatmakeFile{json};

        auto tasks = createTasks(matmakeFile, settings.target);

        for (auto &task : tasks) {
            task->clean();
        }

    } break;
    }

    return 0;
}
