#pragma once

#include "filesystem.h"
#include <vector>

enum class Command {
    Build,
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
    std::string target = "";

    Command command = Command::Build;

    Settings() = default;

    Settings(int argc, char **argv);
};
