#include "settings.h"
#include "exampleproject.h"
#include "os.h"
#include <iostream>
#include <sstream>
#include <thread>

namespace {

const char *helpText = R"_(
usage:
matmake2 [options]

options:
--help -h             print this text
--verbose -v          print extra information
-C [dir]              run in another directory
-j                    set number of worker threads
--backend -b          what build backend to use (ninja, native or makefile)
--target -t [target]  select target (eg gcc, clang, msvc + gcc-debug etc)
--clean               remove all built file
--list                list available targets
--test                run all targets marked with [test]
--compile-commands    output clang compile commands.json
--msvc-wine           setup msvc paths in wine to run in linux

developer options:
--tasks [taskfile]    build a task json-file
--dry-run             only parse matmake file and dump tasklist
--print-tree          print dependency tree
--print-tasks         print list of tasks
--debug -d            print debugging information

)_";

// Because stoi and stol does not seem to work on msvc
long toI(std::string str) {
    std::istringstream ss(str);
    long l;
    ss >> l;
    return l;
}

bool hasNinja() {
    return hasCommand("ninja");
}

bool hasMake() {
    return hasCommand("make");
}

Backend toBackend(std::string str) {
    if (str == "native") {
        return Backend::Native;
    }

    if (str == "makefile") {
        return Backend::Makefile;
    }

    if (str == "ninja") {
        return Backend::Ninja;
    }

    if (str == "default") {
        return Backend::Default;
    }

    std::cerr << str << " is not a valid backend: select one of the following\n"
              << "  native\n  makefile\n  ninja (default)\n";

    std::exit(0);
}

Backend defaultBackend() {
    if (hasNinja()) {
        return Backend::Ninja;
    }

    if (hasMake()) {
        return Backend::Makefile;
    }

    return Backend::Native;
}

} // namespace

Settings::Settings() {
    numThreads = std::thread::hardware_concurrency();
}

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
        else if (arg == "-j") {
            ++i;
            numThreads = toI(args.at(i));
        }
        else if (arg == "--dry-run") {
            skipBuild = true;
        }
        else if (arg == "--target" || arg == "-t") {
            ++i;
            target = args.at(i);
            if (target.find("msvc") != std::string::npos) {
                useMsvcEnvironment = true;
            }
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
        else if (arg == "--msvc-wine") {
            useMsvcEnvironment = true;
        }
        else if (arg == "--backend" || arg == "-b") {
            ++i;
            arg = args.at(i);
            backend = toBackend(arg);
        }
        else if (arg == "--init") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "no path specified, use '.' for current directory"
                          << std::endl;
                std::exit(0);
            }

            createExampleProject(args.at(i));

            std::exit(0);
        }
    }

    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
    }

    if (backend == Backend::Default) {
        backend = defaultBackend();
    }
}
