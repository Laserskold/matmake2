#pragma once

#include "filesystem.h"
#include <map>

enum class SourceType {
    NotFound,

    Header,
    CxxHeader,
    CxxSource,
    ModuleSource,
    ExpandedModuleSource,
    PrecompiledModule,
    Object,
};

inline const auto sourceTypeMap = std::map<filesystem::path, SourceType>{
    {".h", SourceType::Header},
    {".hpp", SourceType::CxxHeader},
    {".hxx", SourceType::CxxHeader},

    {".cpp", SourceType::CxxSource},
    {".cxx", SourceType::CxxSource},
    {".cc", SourceType::CxxSource},

    {".c", SourceType::CxxSource}, // Todo: Add support for separate c-compiler

    {".cppm", SourceType::ModuleSource},
    {".cxxm", SourceType::ModuleSource},

    {".eem", SourceType::ExpandedModuleSource},

    {".pcm", SourceType::PrecompiledModule},

    {".o", SourceType::Object},
    {".obj", SourceType::Object},
};

inline SourceType getType(filesystem::path path) {
    if (auto f = sourceTypeMap.find(path.extension());
        f != sourceTypeMap.end()) {
        return f->second;
    }
    else {
        return SourceType::NotFound;
    }
}
