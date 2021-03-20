#pragma once

enum Os {
    Linux,
    Windows,
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||                 \
    defined(__NT__) || defined(__MINGW32__) || defined(__CYGWIN__)
#define MATMAKE_USING_WINDOWS
#endif
constexpr Os getOs() {
#ifdef MATMAKE_USING_WINDOWS
    return Os::Windows;
#else
    return Os::Linux;
#endif
}

constexpr bool usingMingw() {
#if defined(__MINGW32__) || defined(__CYGWIN__)
    return true;
#else
    return false;
#endif
}