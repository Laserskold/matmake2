//! This file handles all different ways to add to environment
//! Variables

#include "os.h"
#include <cstdlib>
#include <string>

#ifdef MATMAKE_USING_WINDOWS

#if defined(__MINGW32__) || defined(__CYGWIN__)

extern "C" int putenv(char *);

void setenv(const char *name, const char *value, int overwrite) {
    auto arg = std::string{name} + value;
    putenv(arg.data());
}

std::string getEnvVar(std::string name) {
    return getenv(name.c_str());
}

#else

int setenv(const char *name, const char *value, int overwrite) {
    return _putenv_s(name, value);
}

std::string getEnvVar(std::string name) {
    size_t size;
    getenv_s(&size, 0, 0, name.c_str());

    std::string var;
    var.resize(size);
    getenv_s(&size, var.data(), size, name.c_str());

    return var;
}

#endif

#else

std::string getEnvVar(std::string name) {
    return getenv(name.c_str());
}

#endif

void appendEnv(std::string name, std::string value) {
    value = getEnvVar(name) + value;

    setenv(name.c_str(), value.c_str(), 1);
}
