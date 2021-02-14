#pragma once

#include <string>

enum class FlagStyle {
    Inherit,
    Gcc,
    Msvc,
};

enum Os {
    Linux,
    Windows,
};

constexpr Os getOs() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    return Os::Windows;
#else
    return Os::Linux;
#endif
}

std::string translateConfig(std::string config, FlagStyle);

std::string includePrefix(FlagStyle);

std::string extension(std::string ext, FlagStyle);

std::string extensionFromCommandType(std::string command, FlagStyle);
