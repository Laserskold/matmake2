#pragma once

#include "os.h"
#include <string>

enum class FlagStyle {
    Inherit,
    Gcc,
    Msvc,
};

enum class TranslatableString {
    ExeExtension,
    ObjectExtension, // eg. ".o" or ".obj"
    IncludeModuleString,
};

std::string translateConfig(std::string config, FlagStyle);

std::string includePrefix(FlagStyle);

std::string extension(std::string ext, FlagStyle);

std::string extensionFromCommandType(std::string command, FlagStyle);

std::string translateString(TranslatableString, FlagStyle);
