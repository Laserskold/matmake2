#pragma once

#ifdef USE_EXPERIMENTAL_FILESYSTEM

#include <experimental/filesystem>

namespace filesystem = std::experimental::filesystem;

#else

#if __has_include(<filesystem>)

#include <filesystem>

namespace filesystem = std::filesystem;

#else

#include <experimental/filesystem>

namespace filesystem = std::experimental::filesystem;

#endif

#endif
