#pragma once

#include "os.h"

#ifdef USE_EXPERIMENTAL_FILESYSTEM

// Se below

#else

#if __has_include(<filesystem>)

#include <filesystem>

namespace filesystem = std::filesystem;

#else

#define USE_EXPERIMENTAL_FILESYSTEM

#endif // <filesystem>
#endif // USE_EXPERIMENTAL_FILESYSTEM

#ifdef USE_EXPERIMENTAL_FILESYSTEM

#include <experimental/filesystem>

namespace filesystem = std::experimental::filesystem;

//! Compatabilityt function, that only removes the first part of a path,
//! It is enough for the usecases of this project though
inline filesystem::path compat_relative(filesystem::path path,
                                        filesystem::path base) {
    return path.string().substr(base.string().size());
}

#else

inline filesystem::path compat_relative(filesystem::path path,
                                        filesystem::path base) {
    return filesystem::relative(path, base);
}

#endif

#ifdef MATMAKE_USING_WINDOWS

inline filesystem::path normalizePath(std::string path) {
    for (auto &c : path) {
        if (c == '/') {
            c = '\\';
        }
    }
    return filesystem::path{path};
}

inline filesystem::path normalizePath(filesystem::path path) {
    return normalizePath(path.string());
}

#else

inline filesystem::path normalizePath(std::string path) {
    return filesystem::path{path};
}

inline filesystem::path normalizePath(filesystem::path path) {
    return path;
}

#endif
