// copyright Mattias Larsson Sköld 2021

#include "coordinator.h"
#include "createtasks.h"
#include "filesystem.h"
#include "matmakefile.h"
#include "tasklist.h"
#include "json/json.h"

namespace {

const std::string helpText = R"_(
usage:
matmake2 [options]

options:
--help -h             print this text
--verbose -v          print extra information
-C [dir]              run in another directory
--target -t [target]  select target (eg g++, clang++, msvc)
--clean               remove all built file

developer options:
--tasks [taskfile]    build a task json-file
--dry-run             only parse matmake file and dump tasklist
--print-tree          print dependency tree
--print-tasks         print list of tasks


)_";

enum class Command {
    Build,
    ParseTasks,
    Clean,
};

struct Settings {
    filesystem::path taskFile;
    filesystem::path matmakeFile = "matmake.json";
    bool printTree = false;
    bool printTasks = false;
    bool verbose = false;
    bool skipBuild = false;
    std::string target = "g++";

    Command command = Command::Build;

    Settings(int argc, char **argv) {
        std::vector<std::string> args{argv + 1, argv + argc};

        for (size_t i = 0; i < args.size(); ++i) {
            auto arg = args.at(i);

            if (arg == "-h" || arg == "--help") {
                std::cout << helpText;
                std::exit(0);
            }
            else if (arg == "--tasks") {
                ++i;
                arg = args.at(i);

                taskFile = arg;
                command = Command::ParseTasks;
            }
            else if (arg == "--print-tasks") {
                printTasks = true;
            }
            else if (arg == "--print-tree") {
                printTree = true;
            }
            else if (arg == "--verbose" || arg == "-v") {
                verbose = true;
            }
            else if (arg == "-C") {
                ++i;
                filesystem::current_path(args.at(i));
            }
            else if (arg == "--dry-run") {
                skipBuild = true;
            }
            else if (arg == "--target" || arg == "-t") {
                ++i;
                target = args.at(i);
            }
            else if (arg == "--clean") {
                command = Command::Clean;
            }
        }
    }
};

} // namespace

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

                coordinator.execute(*tasks);
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
            coordinator.execute(tasks);
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
