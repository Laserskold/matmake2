#include "defaultfile.h"
#include "os.h"
#include <cstdlib>

namespace {

const char *gccSource = R"_(
  {
    "name": "gcc",
    "in": [ "@all" ],
    "cxx": "g++",
    "ar": "ar",
    "dir": "build/gcc",
    "objdir": "build/.matmake/obj/gcc",
    "command": "[root]",
    "includeprefix": "-I",
    "commands": {
      "cxx": "{c++} -x c++ {src} {modules} -o {out} -c {cxxflags} {flags} {includes}",
      "cc": "{c++} -x c {src} -o {out} -c {cxxflags} {flags} {includes}",
      "exe": "{c++} {in} -o {out} {ldflags} {flags} {includes}",
      "gch": "{c++} {in} -o {out} {depfile} {cxxflags} {flags} {includes}",
      "eem": "{c++} {in} {standard} {includes} {eflags} -E > {out}",
      "pcm": "{c++} {cxxflags} {flags} {includes} {modules} --precompile -x c++-module {src} -o {out} ",
      "cxxm": "{c++} -c {in} -o {out} ",
      "static": "{ar} -rs {out} {in}"
    }
  }

)_";

const char *msvcSource = R"_(
  {
    "name": "msvc",
    "flagstyle": "msvc",
    "in": [ "@all" ],
    "cxx": "cl.exe",
    "ar": "cl.exe",
    "dir": "build/msvc",
    "objdir": "build/.matmake/obj/msvc",
    "command": "[root]",
    "includeprefix": "/I",
    "flags": [ "/EHsc" ],
    "commands": {
      "cxx": "{c++} /TP {src} {modules} /Fo:{out} /c {cxxflags} {flags} {includes}",
      "exe": "{c++} {in}  {ldflags} {flags} {includes} /link /out:{out}",
      "eem": "{c++} /TP {in} {standard} {includes} {eflags} /E > {out}",
      "cxxm": "{c++} /TP {cxxflags} {flags} {includes} -c {in} -o {out} ",
      "static": "{ar} /OUT:{out} {in}"
    }
  }
)_";

bool hasCommand(std::string command) {
    if constexpr (getOs() == Os::Linux) {
        return !system(("command -v " + command + " > /dev/null").c_str());
    }
    else {
        throw std::runtime_error{std::string{__FILE__} + ":" +
                                 std::to_string(__LINE__) +
                                 " is not implemented "};
    }
}

std::string getHighestClang() {
    if (getOs() == Os::Windows) {
        return "clang++.exe";
    }

    for (size_t i = 20; i > 2; --i) {
        auto command = "clang++-" + std::to_string(i);
        if (hasCommand(command)) {
            return command;
        }
    }

    return "clang++";
}

std::string getHighestGcc() {
    if (getOs() == Os::Windows) {
        return "g++.exe";
    }

    for (size_t i = 20; i > 2; --i) {
        auto command = "g++-" + std::to_string(i);
        if (hasCommand(command)) {
            return command;
        }
    }

    return "g++";
}

void normalize(Json &json) {
#ifdef MATMAKE_USING_WINDOWS
    for (auto &c : json.value) {
        if (c == '/') {
            c = '\\';
        }
    }
#endif
}

} // namespace

Json defaultCompiler() {
    auto ret = Json{Json::Array};

    auto createDebugVersion = [](Json json) {
        auto &debugConfig = json["config"];
        debugConfig.push_back({"debug"});
        debugConfig.type = Json::Array;
        json["dir"].value += "-debug";
        json["objdir"].value += "-debug";
        json["name"].value += "-debug";

        return json;
    };

    // Converts a gcc-rule to a clang rule
    auto createClangVersion = [](Json json) {
        json["name"] = "clang";
        json["dir"] = "build/clang";
        json["objdir"] = "build/.matmake/obj/clang";
        json["cxx"] = getHighestClang();
        return json;
    };

    // -- gcc --

    auto gcc = Json::Parse(gccSource);
    gcc["cxx"].value = getHighestGcc();

    ret.reserve(8);

    ret.push_back(gcc);
    ret.push_back(createDebugVersion(gcc));

    // --- clang ---

    auto clang = createClangVersion(gcc);

    ret.push_back(clang);
    ret.push_back(createDebugVersion(clang));

    auto createWineVersion = [](Json json) {
        json["name"].value = "wine-msvc";
        json["dir"].value = "build/wine-msvc";
        json["objdir"] = "build/.matmake/obj/wine-msvc";
        json["cxx"].value = "wine cl.exe";
        json["ar"].value = "wine cl.exe";

        return json;
    };

    // --- msvc and wine ---

    auto msvc = Json::Parse(msvcSource);
    ret.push_back(msvc);
    ret.push_back(createDebugVersion(msvc));

    auto wineMsvc = createWineVersion(msvc);
    ret.push_back(wineMsvc);
    ret.push_back(createDebugVersion(wineMsvc));

    return ret;
}
