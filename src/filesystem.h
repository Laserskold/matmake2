#pragma once

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
inline filesystem::path compat_relative(filesystem::path path, filesystem::path base) {
    return path.string().substr(base.string().size());
}

#else

inline filesystem::path compat_relative(filesystem::path path,
                                        filesystem::path base) {
    return filesystem::relative(path, base);
}

#endif
