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

    Command command = Command::Build;

    Settings();

    Settings(int argc, char **argv);
};
