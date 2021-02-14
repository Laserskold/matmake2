#include "translateconfig.h"
#include <map>

namespace {

const std::map<std::string, std::string> gccConfigs = {
    {"debug", "-g"},
    {"modules", "-fmodules-ts"},
};
const std::map<std::string, std::string> msvcConfigs = {
    {"debug", "/DEBUG"},
    {"modules", "/std:latest"},
};

//! Strings starting with "c++"
std::string translateStandard(std::string config, FlagStyle style) {
    switch (style) {
    case FlagStyle::Msvc:
        return "/std:" + config;
    default: // Gcc
        return "-std=" + config;
    }
}

} // namespace

std::string translateConfig(std::string config, FlagStyle style) {
    if (config.rfind("c++") != std::string::npos) {
        return translateStandard(config, style);
    }

    switch (style) {
    case FlagStyle::Msvc:
        if (auto f = msvcConfigs.find(config); f != msvcConfigs.end()) {
            return f->second;
        }
        break;
    default:
        if (auto f = gccConfigs.find(config); f != gccConfigs.end()) {
            return f->second;
        }
    }
    return {};
}

std::string includePrefix(FlagStyle style) {
    switch (style) {
    case FlagStyle::Msvc:
        return "/I";
    default:
        return "-I";
    }
}
