#include "defaultfile.h"

namespace {

const char *defaultCompilerSource = R"_(
[
  {
    "name": "clang",
    "in": [ "@all" ],
    "cxx": "clang++-11",
    "ar": "ar",
    "dir": "build/clang",
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
  },
  {
    "name": "gcc",
    "in": [ "@all" ],
    "cxx": "g++",
    "ar": "ar",
    "dir": "build/gcc",
    "command": "[root]",
    "includeprefix": "-I",
    "commands": {
      "cxx": "{c++} -x c++ {src} {modules} -o {out} -c {cxxflags} {flags} {includes}",
      "cc": "{c++} -x c {src} {modules} -o {out} -c {cxxflags} {flags} {includes}",
      "exe": "{c++} {in} -o {out} {ldflags} {flags} {includes}",
      "gch": "{c++} {in} -o {out} {depfile} {cxxflags} {flags} {includes}",
      "eem": "{c++} {in} {standard} {includes} {eflags} -E > {out}",
      "pcm": "{c++} {cxxflags} {flags} {includes} {modules} --precompile -x c++-module {src} -o {out} ",
      "cxxm": "{c++} -c {in} -o {out} ",
      "static": "{ar} -rs {out} {in}"
    }
  },
  {
    "name": "wine-msvc",
    "flagstyle": "msvc",
    "in": [ "@all" ],
    "cxx": "wine cl.exe",
    "ar": "wine cl.exe",
    "dir": "build/wine-msvc",
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
  },
  {
    "name": "msvc",
    "flagstyle": "msvc",
    "in": [ "@all" ],
    "cxx": "cl.exe",
    "ar": "cl.exe",
    "dir": "build/msvc",
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
]
)_";

}

Json defaultCompiler() {
    return Json{}.parse(defaultCompilerSource);
}
