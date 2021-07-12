#pragma once

#include "filesystem.h"
#include <vector>

enum class Command {
    Build,
    BuildAndTest,
    ParseTasks,
    Clean,
    List,
};

enum class Backend {
    Default,
    Native,
    Makefile,
    Ninja,
};

struct Settings {
    filesystem::path taskFile;
    bool printTree = false;
    bool printTasks = false;
    bool verbose = false;
    bool debugPrint = false;
    bool skipBuild = false;
    bool outputCompileCommands = false;
    bool useMsvcEnvironment = false;
    std::string target = "";
    size_t numThreads = 0;
    Backend backend = Backend::Default;

    Command command = Command::Build;

    Settings();

    Settings(int argc, char **argv);
};
