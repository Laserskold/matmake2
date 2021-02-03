// copyright Mattias Larsson Sköld 2021

#include "coordinator.h"
#include "filesystem.h"
#include "tasklist.h"

namespace {

const std::string helpText = R"_(
usage:
matmake2 [options]

options:
--help -h             print this text
--tasks [taskfile]    build a task json-file
--print-tree          print dependency tree
--print-tasks         print list of tasks
--verbose -v          print extra information

)_";

struct Settings {
    filesystem::path taskFile;
    bool printTree = false;
    bool printTasks = false;
    bool verbose = false;

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
        }
    }
};

} // namespace

int main(int argc, char **argv) {
    const auto settings = Settings{argc, argv};

    if (!settings.taskFile.empty()) {
        filesystem::current_path(settings.taskFile.parent_path());

        auto tasks = parseTasks(settings.taskFile.filename());

        if (settings.printTasks) {
            printFlat(*tasks);
        }

        if (settings.printTree) {
            std::cout << "\ntreeview\n==================\n";
            auto &root = *tasks->find("@g++");
            root.print(settings.verbose);
        }

        std::cout << "building... \n";

        auto coordinator = Coordinator{};

        coordinator.execute(*tasks);
    }
    else {
        std::cout << "no arguments specified. Run with --help for more info\n";
    }

    return 0;
}
