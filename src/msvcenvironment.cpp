
#include "env.h"
#include "filesystem.h"
#include "os.h"
#include <iostream>
#include <numeric>
#include <vector>

namespace {

#ifdef MATMAKE_USING_WINDOWS

constexpr const char *msvcPathName = "PATH";

#else

constexpr const char *msvcPathName = "WINEPATH";

#endif

//! Get the first folder in the path
filesystem::path getHighest(filesystem::path path) {
    // The first seems to work
    auto it = filesystem::directory_iterator{path};
    if (it == filesystem::directory_iterator{path}) {
        throw std::runtime_error{"could not find subdirectory in " +
                                 path.string()};
    }
    else {
        return *it;
    }
}

//! Concat paths with a leading ";" and also separated by ";"
std::string concatPaths(std::vector<filesystem::path> paths) {
    return std::accumulate(paths.begin(),
                           paths.end(),
                           std::string{},
                           [](std::string &a, filesystem::path &b) {
                               return a + ";" + b.string();
                           });
}

filesystem::path cSpelling() {
    if (usingMingw()) {
        return filesystem::path{"C:/"};
    }
    return filesystem::path{"C:\\"};
}

} // namespace

void setMsvcEnvironment() {
    filesystem::path driveC = [] {
        if (getOs() == Os::Windows) {
            return cSpelling();
        }
        else {
            auto home = filesystem::path{getEnvVar("HOME")};
            return filesystem::path{home / ".wine" / "drive_c"};
        }
    }();

    auto programFiles = [driveC] {
        for (auto &it : filesystem::directory_iterator{driveC}) {
            auto name = it.path().filename().string();
            // Program files folder can be named "Program Files (x86)" or
            // simply "Program (x86)"
            if (name.find("Program") != std::string::npos &&
                name.find("(x86)") != std::string::npos) {
                return it.path();
            }
        }
        throw std::runtime_error{"no program files folder was found in " +
                                 driveC.string()};
    }();

    auto winSdk = programFiles / "Windows Kits" / "10";
    auto vsSdk = getHighest(programFiles / "Microsoft Visual Studio");
    // Guessing Community/Professional and then version number
    auto msvcPath = getHighest(getHighest(vsSdk) / "VC" / "Tools" / "MSVC");

    {
        // WINEPATH/PATH
        auto paths = std::vector{
            //            getHighest(vsSdk) / "Common7" / "IDE", // Required?
            msvcPath / "bin" / "Hostx86" / "x64",
        };

        for (auto &path : paths) {
            if (!filesystem::exists(path)) {
                std::cerr << "cannot find path " << path << "\n";
            }
            path = cSpelling() / compat_relative(path, driveC);
        }

        appendEnv(msvcPathName, concatPaths(paths));
    }

    {
        // INCLUDE
        // example output
        // INCLUDE="$WSDK\Community\VC\Tools\MSVC\14.16.27023\include;$WPSDK\Include\10.0.17763.0\um;
        //          $WPSDK\Include\10.0.17763.0\shared;
        //          $WDXSDK\Include;$WPSDK\Include\10.0.17763.0\ucrt"
        auto winSdkBaseInclude = getHighest(winSdk / "Include");
        auto includes = std::vector{
            msvcPath / "include",
            winSdkBaseInclude / "um",
            winSdkBaseInclude / "shared",
            winSdkBaseInclude / "ucrt",
        };

        for (auto &include : includes) {
            if (!filesystem::exists(include)) {
                std::cerr << "cannot find path " << include << "\n";
            }
            include = cSpelling() / compat_relative(include, driveC);
        }

        appendEnv("INCLUDE", concatPaths(includes));
    }

    {
        // Example output
        // LIB="$WSDK\Community\VC\Tools\MSVC\14.16.27023\lib\x64;
        //      $WPSDK\Lib\10.0.17763.0\um\x64;
        //      $WPSDK\Lib\10.0.17763.0\ucrt\x64"
        auto winSdkBaseLibs = getHighest(winSdk / "Lib");
        auto libPaths = std::vector{
            msvcPath / "lib" / "x64",
            winSdkBaseLibs / "um" / "x64",
            winSdkBaseLibs / "ucrt" / "x64",
        };

        for (auto &lib : libPaths) {
            if (!filesystem::exists(lib)) {
                std::cerr << "cannot find path " << lib << "\n";
            }
            lib = cSpelling() / compat_relative(lib, driveC);
        }

        appendEnv("LIB", concatPaths(libPaths));
    }
}
