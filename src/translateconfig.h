#pragma once

#include <string>

enum class FlagStyle {
    Inherit,
    Gcc,
    Msvc,
};

std::string translateConfig(std::string config, FlagStyle);

std::string includePrefix(FlagStyle);
