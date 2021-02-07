#pragma once

#include "filesystem.h"
#include <vector>

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
    bool debugPrint = false;
    bool skipBuild = false;
    std::string target = "g++";

    Command command = Command::Build;

    Settings(int argc, char **argv);
};
