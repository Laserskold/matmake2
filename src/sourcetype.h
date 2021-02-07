#pragma once

#include "filesystem.h"
#include <map>

enum class SourceType {
    NotFound,

    CxxSource,
    ModuleSource,
    ExpandedModuleSource,
    PrecompiledModule,
    Object,
};

inline const auto sourceTypeMap = std::map<filesystem::path, SourceType>{
    {".cpp", SourceType::CxxSource},
    {".cxx", SourceType::CxxSource},
    {".cc", SourceType::CxxSource},

    {".cppm", SourceType::ModuleSource},
    {".cxxm", SourceType::ModuleSource},

    {".eem", SourceType::ExpandedModuleSource},

    {".pcm", SourceType::PrecompiledModule},

    {".o", SourceType::Object},
};

inline SourceType getType(filesystem::path path) {
    if (auto f = sourceTypeMap.find(path.extension());
        f != sourceTypeMap.end()) {
        //        if (f->second == SourceType::ModuleSource ||
        //            f->second == SourceType::CxxSource) {
        //            if (path.stem().extension() == ".eem") {
        //                return SourceType::ExpandedModuleSource;
        //            }
        //        }
        return f->second;
    }
    else {
        return SourceType::NotFound;
    }
}
