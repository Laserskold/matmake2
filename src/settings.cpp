#include "settings.h"
#include <iostream>

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
--list                list available targets
--test                run all targets marked with [test]
--compile-commands   output clang compile commands.json

developer options:
--tasks [taskfile]    build a task json-file
--dry-run             only parse matmake file and dump tasklist
--print-tree          print dependency tree
--print-tasks         print list of tasks
--debug -d            print debugging information

)_";

} // namespace

Settings::Settings(int argc, char **argv) {
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
        else if (arg == "--debug" || arg == "-d") {
            debugPrint = true;
            verbose = true;
        }
        else if (arg == "--list" || arg == "-l") {
            command = Command::List;
        }
        else if (arg == "--test") {
            command = Command::BuildAndTest;
        }
        else if (arg == "--compile-commands") {
            outputCompileCommands = true;
        }
    }
}
