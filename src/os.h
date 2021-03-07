#pragma once

enum Os {
    Linux,
    Windows,
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define MATMAKE_USING_WINDOWS
#endif
constexpr Os getOs() {
#ifdef MATMAKE_USING_WINDOWS
    return Os::Windows;
#else
    return Os::Linux;
#endif
}
