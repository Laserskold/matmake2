//! This file handles all different ways to add to environment
//! Variables

#include "os.h"
#include <array>
#include <cstdlib>
#include <string>

#ifdef MATMAKE_USING_WINDOWS

#if defined(__MINGW32__) || defined(__CYGWIN__)

extern "C" int putenv(const char *);

void setenv(const char *name, const char *value, int overwrite) {
    auto arg = std::string{name} + value;
    putenv(arg.data());
}

std::string getEnvVar(std::string name) {
    return getenv(name.c_str());
}

#else

#include <Windows.h>

int setenv(const char *name, const char *value, int overwrite) {
    return -!SetEnvironmentVariableA(name, value);
}

std::string getEnvVar(std::string name) {
    auto arr = std::array<char, 9000>{};

    size_t size = GetEnvironmentVariableA(name.c_str(), arr.data(), arr.size());

    return std::string(arr.data(), size);
}

#endif

#else

std::string getEnvVar(std::string name) {
    return getenv(name.c_str());
}

#endif

void appendEnv(std::string name, std::string value) {
    auto oldVar = getEnvVar(name);

    value = oldVar + value;

    setenv(name.c_str(), value.c_str(), 1);
}
